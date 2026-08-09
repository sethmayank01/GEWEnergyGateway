#pragma once

#include "ICloudUploader.h"

#include <string>

#ifdef PLATFORM_ESP32
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#endif

class HttpUploader : public ICloudUploader
{
public:

    explicit HttpUploader(
        const std::string& url);

    ~HttpUploader();

    bool Upload(
        const std::string& json) override;

private:

    std::string m_url;

#ifdef PLATFORM_ESP32

    bool Connect();

    void Disconnect();

    bool IsConnected();

    WiFiClientSecure m_client;

    HTTPClient m_http;

    bool m_connected = false;

#endif

#ifdef PLATFORM_WINDOWS

    std::wstring m_host;
    std::wstring m_path;
    uint16_t m_port = 443;
    bool m_secure = true;

#endif
};