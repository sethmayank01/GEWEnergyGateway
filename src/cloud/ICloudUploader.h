#pragma once

#include <string>
#include <vector>

#include "../models/ServerCommand.h"


class ICloudUploader
{
public:

    virtual ~ICloudUploader() = default;


    virtual bool Upload(
        const std::string& json)=0;
    
      virtual bool UploadLog(
        const std::string& localFile,
        const std::string& remoteName) = 0;


    virtual const std::vector<ServerCommand>&
    GetCommands() const = 0;

    virtual bool ReportCommandStatus(
        uint32_t commandId,
        const std::string& status,
        const std::string& message) = 0;

    virtual bool SendHeartbeat(
        const std::string& firmware,
        bool meterConnected,
        size_t pendingUploads) = 0;

    virtual bool InstallFirmware(
        uint32_t commandId,
        std::string& installedVersion,
        std::string& resultMessage) = 0;

    virtual bool RefreshConfiguration(
        uint32_t commandId,
        std::string& resultMessage) = 0;
};
