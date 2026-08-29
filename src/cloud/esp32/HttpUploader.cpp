#include "../HttpUploader.h"

#include "../../utils/Logger.h"
#include "ServerTrust.h"

#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <Update.h>
#include <mbedtls/sha256.h>

#include <algorithm>
#include <cctype>
#include <memory>
#include <new>

HttpUploader::HttpUploader(
    const std::string& url,
    const std::string& gatewayId,
    const std::string& apiKey)
    : m_url(url),
      m_gatewayId(gatewayId),
      m_apiKey(apiKey)
{
    Logger::Info(
        "ESP32 HttpUploader Created");

    // TODO:
    // Replace with setCACert() when certificate
    // validation is enabled.
    m_client.setCACert(GEW_SERVER_ROOT_CA);
}

HttpUploader::~HttpUploader()
{
    Disconnect();
}

bool HttpUploader::Upload(
    const std::string& json)
{
    //
    // Ensure HTTPS connection exists
    //
    if (!IsConnected())
    {
        Logger::Info(
            "Opening HTTPS connection...");

        Disconnect();

        if (!Connect())
            return false;
    }

    Logger::Info(
        "Uploading JSON to cloud");

    int status =
        m_http.POST(
            (uint8_t*)json.data(),
            json.size());

    //
    // Retry once if connection was lost
    //
    if (status <= 0)
    {
         Logger::Error(
        "HTTP POST failed");

    Logger::Error(
        "Status : " +
        std::to_string(status));

    Logger::Error(
        "Reason : " +
        std::string(
            m_http.errorToString(status).c_str()));
        Disconnect();

        Logger::Info(
            "Reconnecting HTTPS...");

        if (!Connect())
            return false;
        Logger::Info(
    "Client Connected = " +
    std::to_string(
        m_client.connected()));

        status =
            m_http.POST(
                (uint8_t*)json.data(),
                json.size());
        
        Logger::Info(
    "Client Connected After POST = " +
    std::to_string(
        m_client.connected()));
    }

    Logger::Info(
        "HTTP Status: "
        + std::to_string(status));

    std::string response =
        m_http.getString().c_str();

    Logger::Info(
        "Server Response:");

    Logger::Info(
        response);
    
        ParseResponse(response);

    //
    // If server closed connection,
    // prepare for reconnect next upload.
    //
    if (!m_client.connected())
    {
        Logger::Info(
            "Server closed HTTPS connection.");

        Disconnect();
    }
    else
    {
        Logger::Info(
            "HTTPS connection kept alive.");
    }

    return (status == HTTP_CODE_OK);
}

bool HttpUploader::UploadLog(
    const std::string& localFile,
    const std::string& remoteName)
{
    Logger::Info(
        "Uploading log file : " + localFile);

    if (!LittleFS.exists(localFile.c_str()))
    {
        Logger::Warning(
            "Log file not found.");

        return false;
    }

    File file =
        LittleFS.open(
            localFile.c_str(),
            FILE_READ);
    //Mayank
    Logger::Info(
    "File Size = " +
    std::to_string(file.size()));

file.seek(0);

char preview[256];

size_t bytes =
    file.readBytes(
        preview,
        sizeof(preview)-1);

preview[bytes] = '\0';

Serial.println("===== FILE START =====");
Serial.println(preview);
Serial.println("======================");

file.seek(0);
//Mayank
    if (!file)
    {
        Logger::Error(
            "Unable to open log file.");

        return false;
    }

    //
    // Build URL
    //
    std::string url = m_url;

    size_t pos = url.find("upload.php");

    if (pos != std::string::npos)
    {
        url.replace(
            pos,
            strlen("upload.php"),
            "upload_log.php");
    }

    //
    // Add filename as query parameter
    //
    url += "?filename=" + remoteName;

    Logger::Info(
        "Upload URL : " + url);

    HTTPClient http;

    WiFiClientSecure client;

    client.setCACert(GEW_SERVER_ROOT_CA);

    if (!http.begin(
            client,
            url.c_str()))
    {
        Logger::Error(
            "Unable to connect.");

        file.close();

        return false;
    }

    http.addHeader(
        "Content-Type",
        "text/plain");

    http.addHeader(
        "X-Gateway-Id",
        m_gatewayId.c_str());

    http.addHeader(
        "X-Api-Key",
        m_apiKey.c_str());

    http.addHeader(
        "Connection",
        "close");

    Logger::Info(
        "Uploading " +
        std::to_string(file.size()) +
        " bytes");

    //
    // Stream directly from LittleFS
    //
    int status =
        http.sendRequest(
            "POST",
            &file,
            file.size());

    file.close();

    Logger::Info(
        "HTTP Status : " +
        std::to_string(status));

    if (status <= 0)
    {
        Logger::Error(
            http.errorToString(status).c_str());

        http.end();

        return false;
    }

    Logger::Info(
        "Server Response:");

    Logger::Info(
        http.getString().c_str());

    http.end();

    return status == HTTP_CODE_OK;
}

bool HttpUploader::Connect()
{
    if (m_connected)
        return true;

    if (!m_http.begin(
            m_client,
            m_url.c_str()))
    {
        Logger::Error(
            "Unable to establish HTTPS connection.");

        return false;
    }

    //
    // Request Keep-Alive
    //
    m_http.addHeader(
        "Content-Type",
        "application/json");

    m_http.addHeader(
        "Connection",
        "keep-alive");

    m_connected = true;

    Logger::Info(
        "HTTPS connection established.");

    return true;
}

void HttpUploader::Disconnect()
{
    if (!m_connected)
        return;

    Logger::Info(
        "Closing HTTPS connection.");

    m_http.end();

    m_connected = false;
}

bool HttpUploader::IsConnected()
{
    return m_connected;
}

const std::vector<ServerCommand>&
HttpUploader::GetCommands() const
{
    return m_commands;
}

bool HttpUploader::ReportCommandStatus(
    uint32_t commandId,
    const std::string& status,
    const std::string& message)
{
    std::string url = m_url;
    const size_t pos = url.find("upload.php");
    if (pos == std::string::npos)
    {
        Logger::Error("Cannot derive command acknowledgement URL.");
        return false;
    }
    url.replace(pos, strlen("upload.php"), "command_ack.php");

    JsonDocument request;
    request["gatewayId"] = m_gatewayId;
    request["apiKey"] = m_apiKey;
    request["commandId"] = commandId;
    request["status"] = status;
    request["message"] = message;

    String body;
    serializeJson(request, body);
    WiFiClientSecure client;
    // Temporary integration setting. Install the production CA certificate
    // before enabling configuration or firmware update commands.
    client.setCACert(GEW_SERVER_ROOT_CA);
    HTTPClient http;
    http.setConnectTimeout(10000);
    http.setTimeout(15000);
    if (!http.begin(client, url.c_str()))
        return false;

    http.addHeader("Content-Type", "application/json");
    http.addHeader("Connection", "close");
    const int httpStatus = http.POST(body);
    http.end();
    if (httpStatus != HTTP_CODE_OK)
    {
        Logger::Warning("Command status report failed with HTTP " +
                        std::to_string(httpStatus));
        return false;
    }
    return true;
}

bool HttpUploader::SendHeartbeat(
    const std::string& firmware,
    bool meterConnected,
    size_t pendingUploads)
{
    std::string url = m_url;
    const size_t pos = url.find("upload.php");
    if (pos == std::string::npos)
    {
        Logger::Error("Cannot derive heartbeat URL.");
        return false;
    }
    url.replace(pos, strlen("upload.php"), "heartbeat.php");

    JsonDocument request;
    request["gatewayId"] = m_gatewayId;
    request["apiKey"] = m_apiKey;
    request["firmware"] = firmware;
    request["meterConnected"] = meterConnected;
    request["pendingUploads"] = pendingUploads;
    request["wifiRssi"] = WiFi.status() == WL_CONNECTED ? WiFi.RSSI() : 0;
    request["freeHeap"] = ESP.getFreeHeap();
    request["uptimeSeconds"] = millis() / 1000U;

    String body;
    serializeJson(request, body);
    WiFiClientSecure client;
    client.setCACert(GEW_SERVER_ROOT_CA);
    HTTPClient http;
    http.setConnectTimeout(10000);
    http.setTimeout(15000);
    if (!http.begin(client, url.c_str()))
        return false;

    http.addHeader("Content-Type", "application/json");
    http.addHeader("Connection", "close");
    const int httpStatus = http.POST(body);
    const String response = http.getString();
    http.end();
    if (httpStatus != HTTP_CODE_OK)
    {
        Logger::Warning("Heartbeat returned HTTP " + std::to_string(httpStatus));
        return false;
    }

    ParseResponse(std::string(response.c_str()));
    return true;
}

bool HttpUploader::InstallFirmware(
    uint32_t commandId,
    std::string& installedVersion,
    std::string& resultMessage)
{
    std::string manifestUrl = m_url;
    const size_t pos = manifestUrl.find("upload.php");
    if (pos == std::string::npos)
    {
        resultMessage = "Cannot derive firmware manifest URL";
        return false;
    }
    manifestUrl.replace(pos, strlen("upload.php"), "firmware_manifest.php");

    JsonDocument request;
    request["gatewayId"] = m_gatewayId;
    request["apiKey"] = m_apiKey;
    request["commandId"] = commandId;
    String requestBody;
    serializeJson(request, requestBody);

    WiFiClientSecure manifestClient;
    manifestClient.setCACert(GEW_SERVER_ROOT_CA);
    HTTPClient manifestHttp;
    manifestHttp.setConnectTimeout(15000);
    manifestHttp.setTimeout(20000);
    if (!manifestHttp.begin(manifestClient, manifestUrl.c_str()))
    {
        resultMessage = "Unable to initialize firmware manifest request";
        return false;
    }
    manifestHttp.addHeader("Content-Type", "application/json");
    const int manifestStatus = manifestHttp.POST(requestBody);
    const String manifestBody = manifestHttp.getString();
    manifestHttp.end();
    if (manifestStatus != HTTP_CODE_OK)
    {
        resultMessage = "Firmware manifest HTTP " + std::to_string(manifestStatus);
        return false;
    }

    JsonDocument manifest;
    if (deserializeJson(manifest, manifestBody) ||
        !(manifest["success"] | false))
    {
        resultMessage = "Invalid firmware manifest";
        return false;
    }

    const std::string version = manifest["version"] | "";
    const std::string firmwareUrl = manifest["url"] | "";
    std::string expectedSha256 = manifest["sha256"] | "";
    const size_t expectedSize = manifest["size"] | 0U;
    if (version.empty() || expectedSize == 0 || expectedSize > 3264U * 1024U ||
        expectedSha256.size() != 64 ||
        firmwareUrl.rfind("https://www.geworks.co.in/energy/firmware/", 0) != 0)
    {
        resultMessage = "Firmware manifest values are invalid";
        return false;
    }
    std::transform(expectedSha256.begin(), expectedSha256.end(),
                   expectedSha256.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    constexpr size_t OTA_BUFFER_SIZE = 2048;
    constexpr unsigned int OTA_DOWNLOAD_ATTEMPTS = 3;
    std::unique_ptr<uint8_t[]> buffer(
        new (std::nothrow) uint8_t[OTA_BUFFER_SIZE]);
    if (!buffer)
    {
        resultMessage = "Unable to allocate OTA download buffer";
        return false;
    }

    bool firmwareReady = false;
    for (unsigned int attempt = 1;
         attempt <= OTA_DOWNLOAD_ATTEMPTS && !firmwareReady;
         ++attempt)
    {
        Logger::Info("Firmware download attempt " +
                     std::to_string(attempt) + "/" +
                     std::to_string(OTA_DOWNLOAD_ATTEMPTS));

        WiFiClientSecure firmwareClient;
        firmwareClient.setCACert(GEW_SERVER_ROOT_CA);
        firmwareClient.setTimeout(30);
        HTTPClient firmwareHttp;
        firmwareHttp.setConnectTimeout(15000);
        firmwareHttp.setTimeout(30000);
        if (!firmwareHttp.begin(firmwareClient, firmwareUrl.c_str()))
        {
            resultMessage = "Unable to initialize firmware download";
            continue;
        }

        firmwareHttp.addHeader("X-Gateway-Id", m_gatewayId.c_str());
        firmwareHttp.addHeader("X-Api-Key", m_apiKey.c_str());
        firmwareHttp.addHeader("Connection", "close");
        const int downloadStatus = firmwareHttp.GET();
        if (downloadStatus != HTTP_CODE_OK ||
            firmwareHttp.getSize() != static_cast<int>(expectedSize))
        {
            resultMessage = "Firmware download failed or size differs";
            firmwareHttp.end();
            continue;
        }

        if (!Update.begin(expectedSize, U_FLASH))
        {
            resultMessage = "OTA partition cannot accept firmware";
            firmwareHttp.end();
            return false;
        }

        mbedtls_sha256_context sha;
        mbedtls_sha256_init(&sha);
        mbedtls_sha256_starts_ret(&sha, 0);
        WiFiClient* stream = firmwareHttp.getStreamPtr();
        size_t remaining = expectedSize;
        bool writeOk = true;
        while (remaining > 0)
        {
            const size_t chunk = std::min(remaining, OTA_BUFFER_SIZE);
            const size_t received = stream->readBytes(buffer.get(), chunk);
            if (received == 0)
            {
                writeOk = false;
                break;
            }
            mbedtls_sha256_update_ret(&sha, buffer.get(), received);
            if (Update.write(buffer.get(), received) != received)
            {
                writeOk = false;
                break;
            }
            remaining -= received;
        }

        unsigned char digest[32];
        mbedtls_sha256_finish_ret(&sha, digest);
        mbedtls_sha256_free(&sha);
        firmwareHttp.end();

        char actualSha256[65]{};
        for (size_t i = 0; i < sizeof(digest); ++i)
            snprintf(actualSha256 + (i * 2), 3, "%02x", digest[i]);

        if (!writeOk || remaining != 0)
        {
            Update.abort();
            resultMessage = "Firmware download interrupted";
            Logger::Warning(resultMessage);
            continue;
        }

        if (expectedSha256 != actualSha256)
        {
            Update.abort();
            resultMessage = "Firmware SHA-256 verification failed";
            Logger::Warning(resultMessage);
            continue;
        }

        if (!Update.end(true))
        {
            resultMessage = "Unable to finalize OTA image: " +
                            std::string(Update.errorString());
            return false;
        }
        firmwareReady = true;
    }

    if (!firmwareReady)
        return false;

    installedVersion = version;
    resultMessage = "Firmware " + version + " installed; restarting";
    return true;
}

bool HttpUploader::RefreshConfiguration(
    uint32_t commandId,
    std::string& resultMessage)
{
    std::string url = m_url;
    const size_t pos = url.find("upload.php");
    if (pos == std::string::npos)
    {
        resultMessage = "Cannot derive configuration URL";
        return false;
    }
    url.replace(pos, strlen("upload.php"), "gateway_config.php");

    JsonDocument request;
    request["gatewayId"] = m_gatewayId;
    request["apiKey"] = m_apiKey;
    request["commandId"] = commandId;
    String requestBody;
    serializeJson(request, requestBody);

    WiFiClientSecure client;
    client.setCACert(GEW_SERVER_ROOT_CA);
    HTTPClient http;
    http.setConnectTimeout(15000);
    http.setTimeout(20000);
    if (!http.begin(client, url.c_str()))
    {
        resultMessage = "Unable to initialize configuration request";
        return false;
    }
    http.addHeader("Content-Type", "application/json");
    const int status = http.POST(requestBody);
    const String responseBody = http.getString();
    http.end();
    if (status != HTTP_CODE_OK)
    {
        resultMessage = "Configuration request HTTP " + std::to_string(status);
        return false;
    }

    JsonDocument response;
    if (deserializeJson(response, responseBody) ||
        !(response["success"] | false) ||
        !response["configuration"].is<JsonObject>())
    {
        resultMessage = "Configuration response is invalid";
        return false;
    }

    JsonObjectConst configuration =
        response["configuration"].as<JsonObjectConst>();
    const char* gatewayId = configuration["gateway"]["gatewayId"] | "";
    const char* apiKey = configuration["gateway"]["apiKey"] | "";
    const char* manufacturer = configuration["meter"]["manufacturer"] | "";
    const char* model = configuration["meter"]["model"] | "";
    const char* cloudUrl = configuration["cloud"]["url"] | "";
    const int baud = configuration["meter"]["baud"] | 0;
    const int slaveId = configuration["meter"]["slaveId"] | 0;
    const int uploadInterval = configuration["cloud"]["uploadInterval"] | 0;
    if (m_gatewayId != gatewayId || m_apiKey != apiKey ||
        strlen(manufacturer) == 0 || strlen(model) == 0 ||
        strlen(cloudUrl) == 0 || baud <= 0 || slaveId < 1 || slaveId > 247 ||
        uploadInterval <= 0)
    {
        resultMessage = "Configuration validation failed";
        return false;
    }

    File temporary = LittleFS.open("/gateway.json.tmp", "w");
    if (!temporary)
    {
        resultMessage = "Unable to create temporary configuration";
        return false;
    }
    const size_t written = serializeJsonPretty(configuration, temporary);
    temporary.flush();
    temporary.close();
    if (written == 0 || !LittleFS.rename("/gateway.json.tmp", "/gateway.json"))
    {
        LittleFS.remove("/gateway.json.tmp");
        resultMessage = "Unable to activate configuration";
        return false;
    }

    resultMessage = "Gateway configuration updated; restarting";
    return true;
}

void HttpUploader::ParseResponse(
    const std::string& response)
{
    m_commands.clear();

    JsonDocument doc;

    DeserializationError error =
        deserializeJson(doc, response);

    if (error)
    {
        Logger::Warning(
            "Unable to parse server response.");

        return;
    }

    if (!doc["commands"].is<JsonArray>())
    return;

    JsonArray commands =
        doc["commands"].as<JsonArray>();

    for (JsonObject obj : commands)
    {
        ServerCommand command;

        command.id =
            obj["id"] | 0;

        command.command =
            obj["command"] | "";

        if (obj["parameters"].is<JsonObject>())
            serializeJson(obj["parameters"], command.parameters);
        else if (obj["value"].is<const char*>())
        {
            JsonDocument legacy;
            legacy["value"] = obj["value"].as<const char*>();
            serializeJson(legacy, command.parameters);
        }

        m_commands.push_back(command);
    }

    Logger::Info(
        "Commands received : " +
        std::to_string(m_commands.size()));
}
