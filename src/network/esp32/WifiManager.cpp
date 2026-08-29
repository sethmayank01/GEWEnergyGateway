#include "../WiFiManager.h"

#include "../../utils/Logger.h"

#include <WiFi.h>
#include <Preferences.h>
#include <time.h>

namespace
{
    constexpr const char* PROVISIONING_NAMESPACE = "gew_setup";
    constexpr const char* WIFI_COUNT_KEY = "wifi_count";
    constexpr size_t MAX_PROVISIONED_NETWORKS = 8;

    void SynchronizeClock()
    {
        time_t now = 0;
        time(&now);
        if (now > 1700000000)
            return;

        Logger::Info("Synchronizing time...");
        configTime(19800, 0, "pool.ntp.org", "time.nist.gov");
        const uint32_t started = millis();
        while (millis() - started < 15000)
        {
            time(&now);
            if (now > 1700000000)
            {
                Logger::Info("Time synchronized. Unix Time : " +
                             std::to_string(now));
                return;
            }
            delay(500);
        }
        Logger::Warning("Time synchronization timed out.");
    }

    String SsidKey(size_t index)
    {
        return String("ssid") + static_cast<unsigned int>(index);
    }

    String PasswordKey(size_t index)
    {
        return String("pass") + static_cast<unsigned int>(index);
    }

    bool ConnectProvisionedNetworks()
    {
        Preferences preferences;
        if (!preferences.begin(PROVISIONING_NAMESPACE, true))
            return false;

        size_t count = preferences.getUChar(WIFI_COUNT_KEY, 0);
        if (count > MAX_PROVISIONED_NETWORKS)
            count = MAX_PROVISIONED_NETWORKS;

        for (size_t i = 0; i < count; ++i)
        {
            const String ssid = preferences.getString(SsidKey(i).c_str(), "");
            const String password = preferences.getString(PasswordKey(i).c_str(), "");
            if (ssid.isEmpty())
                continue;

            Logger::Info(
                "Runtime WiFi attempt " + std::to_string(i + 1) + "/" +
                std::to_string(count) + ": " + std::string(ssid.c_str()));

            WiFi.mode(WIFI_STA);
            WiFi.begin(ssid.c_str(), password.c_str());
            const uint32_t started = millis();
            while (WiFi.status() != WL_CONNECTED && millis() - started < 10000)
                delay(250);

            if (WiFi.status() == WL_CONNECTED)
            {
                preferences.end();
                return true;
            }

            WiFi.disconnect(false, false);
        }

        preferences.end();
        return false;
    }
}

WiFiManager::WiFiManager(
    const GatewayConfig::WiFiCredential wifi[],
    size_t wifiCount)
    :
    m_wifi(wifi),
    m_wifiCount(wifiCount)
{
}

bool WiFiManager::Connect()
{
    // ProvisioningManager owns credentials on deployed ESP32 gateways. When
    // gateway.json contains no legacy Wi-Fi list, retain/recover that station
    // connection instead of disconnecting it.
    if (m_wifiCount == 0)
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            SynchronizeClock();
            return true;
        }

        const bool connected = ConnectProvisionedNetworks();
        if (connected)
            SynchronizeClock();
        return connected;
    }

    WiFi.mode(WIFI_STA);

    WiFi.disconnect(true);

    delay(500);

    Logger::Info(
        "Connecting to WiFi...");

    for (size_t i = 0; i < m_wifiCount; i++)
    {
        Logger::Info(
            "Trying SSID "
            + std::to_string(i + 1)
            + "/"
            + std::to_string(m_wifiCount)
            + " : "
            + m_wifi[i].ssid);

        WiFi.begin(
            m_wifi[i].ssid.c_str(),
            m_wifi[i].password.c_str());

        uint32_t start = millis();

        while (millis() - start < 10000)
        {
            if (WiFi.status() == WL_CONNECTED)
            {
                Logger::Info(
                    "WiFi Connected.");

                Logger::Info(
                    "SSID : " +
                    std::string(WiFi.SSID().c_str()));

                Logger::Info(
                    "IP   : " +
                    std::string(WiFi.localIP().toString().c_str()));

                Logger::Info(
                    "RSSI : " +
                    std::to_string(WiFi.RSSI()));

                SynchronizeClock();

                return true;
            }

            delay(500);
        }

        Logger::Warning(
            "Failed to connect to "
            + m_wifi[i].ssid);

        WiFi.disconnect(true);

        delay(500);
    }

    Logger::Error(
        "Unable to connect to any configured WiFi.");

    return false;
}

bool WiFiManager::IsConnected() const
{
    return WiFi.status() == WL_CONNECTED;
}

void WiFiManager::MaintainConnection()
{
    if (WiFi.status() == WL_CONNECTED)
        return;

    Logger::Warning(
        "WiFi disconnected.");

    Connect();
}
