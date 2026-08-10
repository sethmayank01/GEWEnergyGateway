#include "../HttpUploader.h"

#include "../../utils/Logger.h"

#include <HTTPClient.h>
#include <ArduinoJson.h>

HttpUploader::HttpUploader(
    const std::string& url)
    : m_url(url)
{
    Logger::Info(
        "ESP32 HttpUploader Created");

    // TODO:
    // Replace with setCACert() when certificate
    // validation is enabled.
    m_client.setInsecure();
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

    client.setInsecure();

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

        command.value =
            obj["value"] | "";

        m_commands.push_back(command);
    }

    Logger::Info(
        "Commands received : " +
        std::to_string(m_commands.size()));
}