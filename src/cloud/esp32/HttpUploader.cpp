#include "../HttpUploader.h"

#include "../../utils/Logger.h"

#include <HTTPClient.h>

HttpUploader::HttpUploader(
    const std::string& url)
    :
    m_url(url)
{
    Logger::Info(
        "ESP32 HttpUploader Created");
}

bool HttpUploader::Upload(
    const std::string& json)
{
    Logger::Info(
        "Uploading JSON to cloud");

    HTTPClient http;

    http.begin(
        m_url.c_str());

    http.addHeader(
        "Content-Type",
        "application/json");

    int status =
        http.POST(
            (uint8_t*)json.c_str(),
            json.length());

    if (status <= 0)
    {
        Logger::Error(
            "HTTP Send Failed");

        http.end();

        return false;
    }

    Logger::Info(
        "HTTP Status: "
        + std::to_string(status));

    std::string response =
        http.getString().c_str();

    Logger::Info(
        "Server Response:");

    Logger::Info(
        response);

    http.end();

    return (status == HTTP_CODE_OK);
}