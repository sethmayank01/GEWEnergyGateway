#pragma once

#include "ICloudUploader.h"
#include "../models/ServerCommand.h"
#include <string>

#ifdef PLATFORM_ESP32
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#endif

class HttpUploader : public ICloudUploader
{
public:

    explicit HttpUploader(
        const std::string& url,
        const std::string& gatewayId,
        const std::string& apiKey);

    ~HttpUploader();

    bool Upload(
        const std::string& json) override;
    
        bool UploadLog(
    const std::string& localFile,
    const std::string& remoteName) override;
    
    const std::vector<ServerCommand>&
GetCommands() const;    

    bool ReportCommandStatus(
        uint32_t commandId,
        const std::string& status,
        const std::string& message) override;

    bool SendHeartbeat(
        const std::string& firmware,
        bool meterConnected,
        size_t pendingUploads) override;

    bool InstallFirmware(
        uint32_t commandId,
        std::string& installedVersion,
        std::string& resultMessage) override;

    bool RefreshConfiguration(
        uint32_t commandId,
        std::string& resultMessage) override;
private:

    std::string m_url;
    std::string m_gatewayId;
    std::string m_apiKey;
    std::vector<ServerCommand> m_commands;

void ParseResponse(
    const std::string& response);

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
