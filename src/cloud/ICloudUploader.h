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
};