#include "../ProvisioningManager.h"

#include "../../utils/Logger.h"
#include "../../models/GatewayConfig.h"
#include "../../models/BuildInfo.h"
#include "../../cloud/esp32/ServerTrust.h"

#if __has_include("../DeviceCredentials.local.h")
#include "../DeviceCredentials.local.h"
#endif

#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <DNSServer.h>
#include <HTTPClient.h>
#include <LittleFS.h>
#include <Preferences.h>
#include <WebServer.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include <algorithm>
#include <string>
#include <vector>
#include <time.h>

namespace
{
#ifndef GEW_FACTORY_RESET_PIN
#define GEW_FACTORY_RESET_PIN -1
#endif
#ifndef GEW_PROVISIONING_LED_PIN
#define GEW_PROVISIONING_LED_PIN 21
#endif
#ifndef GEW_PROVISIONING_URL
#define GEW_PROVISIONING_URL ""
#endif
#ifndef GEW_DEVICE_GATEWAY_ID
#define GEW_DEVICE_GATEWAY_ID ""
#endif
#ifndef GEW_DEVICE_BOOTSTRAP_SECRET
#define GEW_DEVICE_BOOTSTRAP_SECRET ""
#endif
#ifndef GEW_DEVICE_FIRMWARE
#define GEW_DEVICE_FIRMWARE "9.2.0 Prototype"
#endif
#ifndef GEW_DEVICE_HARDWARE
#define GEW_DEVICE_HARDWARE "ESP32-S3"
#endif

    constexpr const char* NVS_NAMESPACE = "gew_setup";
    constexpr const char* NVS_WIFI_COUNT = "wifi_count";
    constexpr const char* NVS_COMMISSIONED = "commissioned";
    constexpr const char* LEGACY_NVS_SSID = "wifi_ssid";
    constexpr const char* LEGACY_NVS_PASSWORD = "wifi_pass";
    constexpr size_t MAX_SAVED_WIFI_NETWORKS = 8;
    constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS = 15000;
    constexpr uint16_t DNS_PORT = 53;
    constexpr uint32_t FACTORY_RESET_HOLD_MS = 8000;
    constexpr uint8_t SETUP_AP_CHANNEL = 6;
    constexpr size_t MAX_PROVISIONING_RESPONSE_BYTES = 16 * 1024;
    constexpr unsigned int PROVISIONING_DOWNLOAD_ATTEMPTS = 3;
    constexpr uint32_t COMMISSIONING_BLINK_MS = 500;

    // Match the known-working GatewayHealth LED implementation used by this
    // gateway hardware (GPIO 21 and RGB byte order).
    Adafruit_NeoPixel provisioningLed(
        1,
        GEW_PROVISIONING_LED_PIN,
        NEO_RGB + NEO_KHZ800);
    TaskHandle_t commissioningLedTask = nullptr;
    volatile uint8_t commissioningRed = 255;
    volatile uint8_t commissioningGreen = 0;
    volatile uint8_t commissioningBlue = 0;

    void SetProvisioningLed(uint8_t red, uint8_t green, uint8_t blue)
    {
        if (GEW_PROVISIONING_LED_PIN < 0)
            return;

        provisioningLed.setPixelColor(
            0,
            provisioningLed.Color(red, green, blue));
        provisioningLed.show();
    }

    void SetCommissioningLedColor(uint8_t red, uint8_t green, uint8_t blue)
    {
        commissioningRed = red;
        commissioningGreen = green;
        commissioningBlue = blue;
    }

    void CommissioningLedTask(void*)
    {
        bool illuminated = false;
        for (;;)
        {
            illuminated = !illuminated;
            if (illuminated)
            {
                SetProvisioningLed(
                    commissioningRed,
                    commissioningGreen,
                    commissioningBlue);
            }
            else
            {
                SetProvisioningLed(0, 0, 0);
            }
            vTaskDelay(pdMS_TO_TICKS(COMMISSIONING_BLINK_MS));
        }
    }

    void StopCommissioningLed(uint8_t red, uint8_t green, uint8_t blue)
    {
        if (commissioningLedTask != nullptr)
        {
            vTaskDelete(commissioningLedTask);
            commissioningLedTask = nullptr;
        }
        SetProvisioningLed(red, green, blue);
    }

    void InitializeProvisioningLed()
    {
        if (GEW_PROVISIONING_LED_PIN < 0)
            return;

        provisioningLed.begin();
        provisioningLed.setBrightness(40);
        provisioningLed.clear();
        provisioningLed.show();
        SetCommissioningLedColor(255, 0, 0); // Blinking red: commissioning.
        if (xTaskCreate(
                CommissioningLedTask,
                "commissioning-led",
                2048,
                nullptr,
                1,
                &commissioningLedTask) != pdPASS)
        {
            commissioningLedTask = nullptr;
            SetProvisioningLed(255, 0, 0);
        }
    }

    const char SETUP_PAGE[] PROGMEM = R"HTML(
<!doctype html><html><head><meta name="viewport" content="width=device-width,initial-scale=1">
<title>GEW Gateway Setup</title><style>
body{font-family:system-ui,sans-serif;background:#f3f5f7;margin:0;padding:18px;color:#17202a}
main{max-width:620px;margin:auto;background:#fff;padding:22px;border-radius:14px;box-shadow:0 5px 24px #0002}
.network{padding:12px 0;border-top:1px solid #dde3e8}.name{font-weight:650}.signal{float:right;color:#58636d}
input,button{box-sizing:border-box;width:100%;padding:12px;margin-top:8px;font-size:16px}
button{background:#075e54;color:white;border:0;border-radius:7px;font-weight:600;margin-top:18px}#status{min-height:24px;margin-top:15px}
</style></head><body><main><h2>GEW Gateway Setup</h2>
<p>Enter passwords for every network this gateway may use. Blank networks will not be saved.</p>
<div id="networks">Scanning networks...</div>
<button id="submit" onclick="connectWifi()">Verify networks and finish setup</button>
<div id="status"></div><script>
const statusEl=document.getElementById('status');
async function scan(){try{const r=await fetch('/api/networks');const a=await r.json();const box=document.getElementById('networks');box.innerHTML='';
a.forEach(n=>{const row=document.createElement('div');row.className='network';const label=document.createElement('div');label.className='name';label.textContent=n.ssid;
const signal=document.createElement('span');signal.className='signal';signal.textContent=n.rssi+' dBm';label.appendChild(signal);
const input=document.createElement('input');input.type='password';input.placeholder='Password (leave blank to skip)';input.dataset.ssid=n.ssid;input.dataset.rssi=n.rssi;
row.appendChild(label);row.appendChild(input);box.appendChild(row);});if(!a.length)box.textContent='No Wi-Fi networks detected. Restart the gateway to scan again.';
}catch(e){statusEl.textContent='Unable to load networks. Reconnect to the setup Wi-Fi and reload.'}}
async function connectWifi(){const networks=[...document.querySelectorAll('input[data-ssid]')].filter(x=>x.value.length>0).map(x=>({ssid:x.dataset.ssid,password:x.value,rssi:Number(x.dataset.rssi)}));
if(!networks.length){statusEl.textContent='Enter a password for at least one network.';return}document.getElementById('submit').disabled=true;
statusEl.textContent='Testing '+networks.length+' network(s). Keep this page open; this can take a short while.';
try{const r=await fetch('/api/connect',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({networks})});const x=await r.json();statusEl.textContent=x.message;if(!r.ok)document.getElementById('submit').disabled=false;
}catch(e){statusEl.textContent='The gateway is reconnecting to the strongest verified network. You may close this page.'}}
scan();</script></main></body></html>)HTML";

    struct VerifiedWiFi
    {
        String ssid;
        String password;
        int32_t rssi;
    };

    std::string JsonEscape(const String& value)
    {
        std::string result;
        result.reserve(value.length());
        for (size_t i = 0; i < value.length(); ++i)
        {
            const char c = value[i];
            if (c == '"' || c == '\\')
                result.push_back('\\');
            if (static_cast<unsigned char>(c) >= 0x20)
                result.push_back(c);
        }
        return result;
    }

    bool ConnectToWiFi(const String& ssid, const String& password)
    {
        WiFi.mode(WIFI_AP_STA);
        WiFi.begin(ssid.c_str(), password.c_str());

        const uint32_t started = millis();
        while (WiFi.status() != WL_CONNECTED &&
               millis() - started < WIFI_CONNECT_TIMEOUT_MS)
        {
            delay(250);
        }

        if (WiFi.status() == WL_CONNECTED)
            SetCommissioningLedColor(0, 160, 255); // Blinking blue: site Wi-Fi connected.

        return WiFi.status() == WL_CONNECTED;
    }

    String WiFiSsidKey(size_t index)
    {
        return String("ssid") + static_cast<unsigned int>(index);
    }

    String WiFiPasswordKey(size_t index)
    {
        return String("pass") + static_cast<unsigned int>(index);
    }

    size_t LoadWiFiCount(Preferences& preferences)
    {
        size_t count = preferences.getUChar(NVS_WIFI_COUNT, 0);
        if (count > MAX_SAVED_WIFI_NETWORKS)
            count = MAX_SAVED_WIFI_NETWORKS;

        // Migrate credentials saved by the first single-network prototype.
        if (count == 0 && preferences.isKey(LEGACY_NVS_SSID))
        {
            const String legacySsid = preferences.getString(LEGACY_NVS_SSID, "");
            if (!legacySsid.isEmpty())
            {
                preferences.putString(WiFiSsidKey(0).c_str(), legacySsid);
                preferences.putString(
                    WiFiPasswordKey(0).c_str(),
                    preferences.getString(LEGACY_NVS_PASSWORD, ""));
                preferences.putUChar(NVS_WIFI_COUNT, 1);
                count = 1;
            }
            preferences.remove(LEGACY_NVS_SSID);
            preferences.remove(LEGACY_NVS_PASSWORD);
        }

        return count;
    }

    bool SaveWiFiCredential(
        Preferences& preferences,
        const String& ssid,
        const String& password)
    {
        size_t count = LoadWiFiCount(preferences);
        size_t index = count;

        for (size_t i = 0; i < count; ++i)
        {
            if (preferences.getString(WiFiSsidKey(i).c_str(), "") == ssid)
            {
                index = i;
                break;
            }
        }

        if (index == count)
        {
            if (count >= MAX_SAVED_WIFI_NETWORKS)
                return false;
            ++count;
            preferences.putUChar(NVS_WIFI_COUNT, static_cast<uint8_t>(count));
        }

        const bool ssidSaved =
            preferences.putString(WiFiSsidKey(index).c_str(), ssid) > 0;
        // An empty password is valid for an open network and may report zero
        // bytes written, so the SSID write is the authoritative result.
        preferences.putString(WiFiPasswordKey(index).c_str(), password);
        return ssidSaved;
    }

    void ClearSavedWiFiCredentials(Preferences& preferences)
    {
        for (size_t i = 0; i < MAX_SAVED_WIFI_NETWORKS; ++i)
        {
            preferences.remove(WiFiSsidKey(i).c_str());
            preferences.remove(WiFiPasswordKey(i).c_str());
        }
        preferences.putUChar(NVS_WIFI_COUNT, 0);
        preferences.remove(LEGACY_NVS_SSID);
        preferences.remove(LEGACY_NVS_PASSWORD);
    }

    bool ConnectToSavedWiFi(Preferences& preferences)
    {
        const size_t count = LoadWiFiCount(preferences);
        for (size_t i = 0; i < count; ++i)
        {
            const String ssid =
                preferences.getString(WiFiSsidKey(i).c_str(), "");
            const String password =
                preferences.getString(WiFiPasswordKey(i).c_str(), "");
            if (ssid.isEmpty())
                continue;

            Logger::Info(
                "Trying saved WiFi " + std::to_string(i + 1) + "/" +
                std::to_string(count) + ": " + std::string(ssid.c_str()));

            if (ConnectToWiFi(ssid, password))
                return true;

            WiFi.disconnect(false, false);
            delay(250);
        }

        return false;
    }

    String SetupAccessPointName()
    {
        const uint64_t chipId = ESP.getEfuseMac();
        char suffix[7]{};
        snprintf(suffix, sizeof(suffix), "%06llX",
                 static_cast<unsigned long long>(chipId & 0xFFFFFFULL));
        return String("GEW-Gateway-") + suffix;
    }

    bool FactoryResetRequested()
    {
        if (GEW_FACTORY_RESET_PIN < 0)
            return false;

        pinMode(GEW_FACTORY_RESET_PIN, INPUT_PULLUP);
        if (digitalRead(GEW_FACTORY_RESET_PIN) != LOW)
            return false;

        const uint32_t started = millis();
        while (digitalRead(GEW_FACTORY_RESET_PIN) == LOW)
        {
            if (millis() - started >= FACTORY_RESET_HOLD_MS)
                return true;
            delay(25);
        }
        return false;
    }

    bool IsProvisioningIdentityConfigured()
    {
        return strlen(GEW_PROVISIONING_URL) > 0 &&
               strlen(GEW_DEVICE_GATEWAY_ID) > 0 &&
               strlen(GEW_DEVICE_BOOTSTRAP_SECRET) > 0;
    }

    bool SynchronizeProvisioningClock()
    {
        time_t now = 0;
        time(&now);
        if (now > 1700000000)
            return true;

        Logger::Info("Synchronizing time before secure provisioning...");
        configTime(19800, 0, "pool.ntp.org", "time.nist.gov");
        const uint32_t started = millis();
        while (millis() - started < 15000)
        {
            time(&now);
            if (now > 1700000000)
                return true;
            delay(500);
        }
        Logger::Error("Time synchronization failed; secure provisioning stopped.");
        return false;
    }

    bool ValidateConfiguration(JsonObjectConst configuration)
    {
        const char* gatewayId = configuration["gateway"]["gatewayId"] | "";
        const char* apiKey = configuration["gateway"]["apiKey"] | "";
        const char* manufacturer = configuration["meter"]["manufacturer"] | "";
        const char* model = configuration["meter"]["model"] | "";
        const int baud = configuration["meter"]["baud"] | 0;
        const int slaveId = configuration["meter"]["slaveId"] | 0;
        const char* cloudUrl = configuration["cloud"]["url"] | "";
        const int uploadInterval = configuration["cloud"]["uploadInterval"] | 0;

        if (strcmp(gatewayId, GEW_DEVICE_GATEWAY_ID) != 0)
        {
            Logger::Error("Provisioning response gateway ID does not match this device.");
            return false;
        }

        if (strlen(apiKey) == 0 || strlen(manufacturer) == 0 ||
            strlen(model) == 0 || strlen(cloudUrl) == 0 ||
            baud <= 0 || slaveId < 1 || slaveId > 247 || uploadInterval <= 0)
        {
            Logger::Error("Provisioning response configuration is incomplete.");
            return false;
        }

        return true;
    }

    bool SaveConfiguration(JsonObjectConst configuration)
    {
        if (!LittleFS.begin(false) && !LittleFS.begin(true))
        {
            Logger::Error("Unable to mount LittleFS for provisioning.");
            return false;
        }

        File temporary = LittleFS.open("/gateway.json.tmp", "w");
        if (!temporary)
        {
            Logger::Error("Unable to create temporary gateway configuration.");
            return false;
        }

        const size_t written = serializeJsonPretty(configuration, temporary);
        temporary.flush();
        temporary.close();
        if (written == 0)
        {
            LittleFS.remove("/gateway.json.tmp");
            Logger::Error("Unable to write gateway configuration.");
            return false;
        }

        if (!LittleFS.rename("/gateway.json.tmp", CONFIG_FILE))
        {
            LittleFS.remove("/gateway.json.tmp");
            Logger::Error("Unable to activate gateway configuration.");
            return false;
        }

        Logger::Info("Gateway configuration saved.");
        return true;
    }

    void MarkGatewayCommissioned()
    {
        Preferences state;
        if (state.begin(NVS_NAMESPACE, false))
        {
            state.putBool(NVS_COMMISSIONED, true);
            state.end();
        }
    }

    bool DownloadConfigurationOnce()
    {
        if (!IsProvisioningIdentityConfigured())
        {
            Logger::Error("Manufacturing identity/provisioning URL is not configured.");
            return false;
        }

        if (!SynchronizeProvisioningClock())
            return false;

        Logger::Info("Requesting gateway configuration from provisioning server...");

        JsonDocument requestDocument;
        requestDocument["gatewayId"] = GEW_DEVICE_GATEWAY_ID;
        requestDocument["macAddress"] = WiFi.macAddress();
        requestDocument["bootstrapSecret"] = GEW_DEVICE_BOOTSTRAP_SECRET;
        requestDocument["firmware"] = GEW_FIRMWARE_VERSION;
        requestDocument["hardware"] = GEW_DEVICE_HARDWARE;
        JsonArray wifiNetworks = requestDocument["wifiNetworks"].to<JsonArray>();
        Preferences wifiState;
        if (wifiState.begin(NVS_NAMESPACE, true))
        {
            const size_t count = LoadWiFiCount(wifiState);
            for (size_t i = 0; i < count; ++i)
            {
                const String ssid = wifiState.getString(WiFiSsidKey(i).c_str(), "");
                if (!ssid.isEmpty())
                    wifiNetworks.add(ssid);
            }
            wifiState.end();
        }

        String requestBody;
        serializeJson(requestDocument, requestBody);

        WiFiClientSecure client;
        client.setCACert(GEW_SERVER_ROOT_CA);

        HTTPClient http;
        http.setConnectTimeout(15000);
        http.setTimeout(20000);
        if (!http.begin(client, GEW_PROVISIONING_URL))
        {
            Logger::Error("Unable to initialize provisioning request.");
            return false;
        }

        http.addHeader("Content-Type", "application/json");
        http.addHeader("Connection", "close");
        const int status = http.POST(requestBody);

        if (status != HTTP_CODE_OK)
        {
            Logger::Error("Provisioning server returned HTTP status " +
                          std::to_string(status));
            http.end();
            return false;
        }

        const int responseSize = http.getSize();
        if (responseSize > static_cast<int>(MAX_PROVISIONING_RESPONSE_BYTES))
        {
            Logger::Error("Provisioning response is too large.");
            http.end();
            return false;
        }

        const String responseBody = http.getString();
        http.end();
        if (responseBody.isEmpty() ||
            responseBody.length() > MAX_PROVISIONING_RESPONSE_BYTES)
        {
            Logger::Error("Provisioning server returned an empty or oversized response.");
            return false;
        }

        JsonDocument responseDocument;
        const DeserializationError error =
            deserializeJson(responseDocument, responseBody);
        if (error)
        {
            Logger::Error("Provisioning response is not valid JSON.");
            return false;
        }

        if (!(responseDocument["success"] | false))
        {
            const char* message = responseDocument["message"] | "Provisioning rejected.";
            Logger::Error(message);
            return false;
        }

        JsonObjectConst configuration =
            responseDocument["configuration"].as<JsonObjectConst>();
        if (configuration.isNull() || !ValidateConfiguration(configuration))
            return false;

        if (!SaveConfiguration(configuration))
            return false;

        MarkGatewayCommissioned();
        return true;
    }

    bool DownloadConfiguration()
    {
        SetCommissioningLedColor(0, 255, 180); // Blinking teal: contacting server.
        for (unsigned int attempt = 1;
             attempt <= PROVISIONING_DOWNLOAD_ATTEMPTS;
             ++attempt)
        {
            Logger::Info("Gateway provisioning attempt " +
                         std::to_string(attempt) + "/" +
                         std::to_string(PROVISIONING_DOWNLOAD_ATTEMPTS));
            if (DownloadConfigurationOnce())
                return true;

            if (attempt < PROVISIONING_DOWNLOAD_ATTEMPTS)
                Logger::Warning("Gateway provisioning attempt failed; retrying.");
        }
        return false;
    }

    bool EnsureGatewayConfiguration()
    {
        if (!LittleFS.begin(false) && !LittleFS.begin(true))
        {
            Logger::Error("Unable to mount LittleFS.");
            return false;
        }

        File existing = LittleFS.open(CONFIG_FILE, "r");
        if (existing)
        {
            existing.close();
            MarkGatewayCommissioned();
            StopCommissioningLed(0, 255, 0);
            return true;
        }

        Logger::Info("Gateway configuration is missing; contacting server.");
        return DownloadConfiguration();
    }
}

bool ProvisioningManager::EnsureProvisioned()
{
    InitializeProvisioningLed();

    Preferences preferences;
    if (!preferences.begin(NVS_NAMESPACE, false))
    {
        Logger::Error("Unable to open provisioning storage.");
        return false;
    }

    if (FactoryResetRequested())
    {
        Logger::Warning("Factory reset requested; clearing customer provisioning.");
        preferences.clear();
        if (LittleFS.begin())
        {
            LittleFS.remove("/gateway.json");
            LittleFS.remove("/gateway.json.tmp");
        }
    }

    const size_t savedWiFiCount = LoadWiFiCount(preferences);
    if (savedWiFiCount > 0)
    {
        Logger::Info("Connecting with saved WiFi credentials...");
        if (ConnectToSavedWiFi(preferences))
        {
            Logger::Info("WiFi connected: " + std::string(WiFi.localIP().toString().c_str()));
            preferences.end();
            const bool configured = EnsureGatewayConfiguration();
            if (configured)
                StopCommissioningLed(0, 255, 0);
            return configured;
        }
        Logger::Warning("Unable to connect to any saved WiFi network.");
        WiFi.disconnect(false, false);

        if (preferences.getBool(NVS_COMMISSIONED, false))
        {
            Logger::Warning(
                "Gateway is already commissioned; continuing offline and retaining queued data.");
            preferences.end();
            StopCommissioningLed(255, 0, 255);
            return true;
        }

        Logger::Warning("Gateway is not commissioned; starting setup portal.");
    }

    const String apName = SetupAccessPointName();
    WiFi.persistent(false);
    WiFi.disconnect(true, false);
    WiFi.softAPdisconnect(true);
    delay(250);
    WiFi.mode(WIFI_STA);

    Logger::Info("Running WiFi radio scan before starting setup AP...");
    const int diagnosticNetworkCount = WiFi.scanNetworks(false, true);
    std::string networkJson = "[";
    Logger::Info("WiFi networks detected: " +
                 std::to_string(diagnosticNetworkCount < 0 ? 0 : diagnosticNetworkCount));
    for (int i = 0; i < diagnosticNetworkCount; ++i)
    {
        if (i > 0)
            networkJson += ',';

        networkJson += "{\"ssid\":\"" + JsonEscape(WiFi.SSID(i)) +
                       "\",\"rssi\":" + std::to_string(WiFi.RSSI(i)) +
                       ",\"secure\":" +
                       (WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "false" : "true") + "}";

        if (i < 5)
        {
            Logger::Info("Detected WiFi: " + std::string(WiFi.SSID(i).c_str()) +
                         " (" + std::to_string(WiFi.RSSI(i)) + " dBm)");
        }
    }
    networkJson += ']';
    WiFi.scanDelete();

    WiFi.mode(WIFI_AP);
    delay(250);

    // The setup AP is open for this provisioning milestone. Production units
    // should use a unique setup password printed on the device label.
    if (!WiFi.softAP(apName.c_str(), nullptr, SETUP_AP_CHANNEL, false, 4))
    {
        Logger::Error("Unable to start setup access point.");
        preferences.end();
        return false;
    }

    // Configure the AP network only after softAP() creates the interface.
    // On this ESP32 core, doing it before AP creation—or relying on DHCP after
    // a station scan—can leave clients associated without a DHCP lease.
    const IPAddress apIp(192, 168, 4, 1);
    const IPAddress netmask(255, 255, 255, 0);
    if (!WiFi.softAPConfig(apIp, apIp, netmask))
    {
        Logger::Error("Unable to start setup WiFi DHCP service.");
        WiFi.softAPdisconnect(true);
        preferences.end();
        return false;
    }

    DNSServer dns;
    WebServer server(80);
    dns.start(DNS_PORT, "*", WiFi.softAPIP());

    Logger::Info("Setup WiFi started: " + std::string(apName.c_str()));
    Logger::Info("Setup WiFi security: OPEN (provisioning test)");
    Logger::Info("Setup WiFi MAC: " + std::string(WiFi.softAPmacAddress().c_str()));
    Logger::Info("Setup WiFi channel: " + std::to_string(SETUP_AP_CHANNEL) + " (2.4 GHz)");
    Logger::Info("Setup WiFi IP: " + std::string(WiFi.softAPIP().toString().c_str()));
    Logger::Info("Open http://192.168.4.1 to configure the gateway.");

    bool connected = false;
    bool setupClientPresent = false;

    server.on("/", HTTP_GET, [&server]() {
        server.send_P(200, "text/html", SETUP_PAGE);
    });

    server.on("/api/networks", HTTP_GET, [&server, &networkJson]() {
        server.send(200, "application/json", networkJson.c_str());
    });

    // Common captive-portal probes. Keeping these on the local HTTP server
    // prevents client operating systems from repeatedly probing unknown paths.
    server.on("/hotspot-detect.html", HTTP_GET, [&server]() {
        server.send_P(200, "text/html", SETUP_PAGE);
    });
    server.on("/generate_204", HTTP_GET, [&server]() {
        server.sendHeader("Location", "http://192.168.4.1", true);
        server.send(302, "text/plain", "");
    });
    server.on("/connecttest.txt", HTTP_GET, [&server]() {
        server.sendHeader("Location", "http://192.168.4.1", true);
        server.send(302, "text/plain", "");
    });
    server.on("/ncsi.txt", HTTP_GET, [&server]() {
        server.sendHeader("Location", "http://192.168.4.1", true);
        server.send(302, "text/plain", "");
    });
    server.on("/favicon.ico", HTTP_GET, [&server]() {
        server.send(204, "text/plain", "");
    });

    server.on("/api/connect", HTTP_POST, [&]() {
        JsonDocument submitted;
        if (deserializeJson(submitted, server.arg("plain")) ||
            !submitted["networks"].is<JsonArray>())
        {
            server.send(400, "application/json",
                        "{\"connected\":false,\"message\":\"Invalid Wi-Fi list.\"}");
            return;
        }

        JsonArray networks = submitted["networks"].as<JsonArray>();
        if (networks.size() == 0 || networks.size() > MAX_SAVED_WIFI_NETWORKS)
        {
            server.send(400, "application/json",
                        "{\"connected\":false,\"message\":\"Select between 1 and 8 networks.\"}");
            return;
        }

        std::vector<VerifiedWiFi> verified;
        for (JsonObjectConst network : networks)
        {
            const String ssid = network["ssid"] | "";
            const String password = network["password"] | "";
            if (ssid.isEmpty() || ssid.length() > 32 ||
                password.isEmpty() || password.length() > 63)
                continue;

            Logger::Info("Testing setup WiFi: " + std::string(ssid.c_str()));
            WiFi.disconnect(false, false);
            if (ConnectToWiFi(ssid, password))
            {
                const int32_t actualRssi = WiFi.RSSI();
                verified.push_back({ssid, password, actualRssi});
                Logger::Info("Verified setup WiFi: " + std::string(ssid.c_str()) +
                             " (" + std::to_string(actualRssi) + " dBm)");
            }
            else
            {
                Logger::Warning("Unable to verify setup WiFi: " +
                                std::string(ssid.c_str()));
            }
        }

        if (verified.empty())
        {
            WiFi.disconnect(false, false);
            server.send(401, "application/json",
                        "{\"connected\":false,\"message\":\"None of the networks connected. Check the passwords and try again.\"}");
            return;
        }

        std::sort(verified.begin(), verified.end(),
                  [](const VerifiedWiFi& left, const VerifiedWiFi& right) {
                      return left.rssi > right.rssi;
                  });

        ClearSavedWiFiCredentials(preferences);
        for (const VerifiedWiFi& network : verified)
        {
            if (!SaveWiFiCredential(preferences, network.ssid, network.password))
            {
                server.send(507, "application/json",
                            "{\"connected\":false,\"message\":\"Unable to save the verified Wi-Fi list.\"}");
                return;
            }
        }

        // Keep the saved order strongest-first for this restart and all future
        // reconnect attempts. Passwords never leave NVS on the ESP32.
        WiFi.disconnect(false, false);
        ConnectToWiFi(verified.front().ssid, verified.front().password);
        connected = true;
        Logger::Info("Verified and saved " + std::to_string(verified.size()) +
                     " WiFi network(s), strongest first.");
        server.send(200, "application/json",
                    "{\"connected\":true,\"message\":\"Networks verified. Gateway is restarting on the strongest connection.\"}");

        // Restart from the successful request itself. Shutting down the AP and
        // web server first can stall while clients still have open sockets.
        delay(1500);
        Logger::Info("WiFi provisioning completed. Restarting gateway...");
        Logger::Flush();
        delay(100);
        ESP.restart();
    });

    server.on("/api/status", HTTP_GET, [&]() {
        const std::string response =
            "{\"connected\":" + std::string(connected ? "true" : "false") +
            ",\"savedNetworks\":" + std::to_string(LoadWiFiCount(preferences)) +
            ",\"ip\":\"" + JsonEscape(WiFi.localIP().toString()) + "\"}";
        server.send(200, "application/json", response.c_str());
    });

    server.onNotFound([&server]() {
        server.sendHeader("Location", "http://192.168.4.1", true);
        server.send(302, "text/plain", "");
    });

    server.begin();
    while (!connected)
    {
        dns.processNextRequest();
        server.handleClient();

        const bool clientPresent = WiFi.softAPgetStationNum() > 0;
        if (clientPresent != setupClientPresent)
        {
            setupClientPresent = clientPresent;
            if (setupClientPresent)
            {
                SetCommissioningLedColor(255, 180, 0); // Blinking yellow: setup client connected.
                Logger::Info("Client connected to setup WiFi.");
            }
            else
            {
                SetCommissioningLedColor(255, 0, 0);
                Logger::Info("Client disconnected from setup WiFi.");
            }
        }
        delay(2);
    }

    // Give the browser time to receive the success response before stopping AP.
    const uint32_t completedAt = millis();
    while (millis() - completedAt < 1000)
    {
        dns.processNextRequest();
        server.handleClient();
        delay(2);
    }

    // Normally unreachable because a successful request restarts the device.
    server.stop();
    dns.stop();
    preferences.end();
    return false;
}
