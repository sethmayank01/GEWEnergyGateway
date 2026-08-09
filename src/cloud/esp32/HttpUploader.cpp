#include "../HttpUploader.h"

#include "../../utils/Logger.h"

#include <HTTPClient.h>

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
        Logger::Warning(
            "HTTPS connection lost.");

        Disconnect();

        Logger::Info(
            "Reconnecting HTTPS...");

        if (!Connect())
            return false;

        status =
            m_http.POST(
                (uint8_t*)json.data(),
                json.size());
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
    return m_connected &&
           m_client.connected();
}