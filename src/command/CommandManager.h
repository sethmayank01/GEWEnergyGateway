#pragma once

#include <vector>

#include "../models/ServerCommand.h"
#include "../cloud/ICloudUploader.h"

class CommandManager
{
public:

    explicit CommandManager(
        ICloudUploader& uploader);

    void Process(
        const std::vector<ServerCommand>& commands);

private:

    void ProcessCommand(
        const ServerCommand& command);

private:

    ICloudUploader& m_uploader;
};