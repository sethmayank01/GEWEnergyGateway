#include "../../cloud/HttpUploader.h"

#include "../../utils/Logger.h"

HttpUploader::HttpUploader(
    const std::string& url)
    : m_url(url)
{
    Logger::Info("ESP32 HttpUploader Created");
}

bool HttpUploader::Upload(
    const std::string& json)
{
    (void)json;

    Logger::Warning(
        "ESP32 HttpUploader not implemented.");

    return false;
}