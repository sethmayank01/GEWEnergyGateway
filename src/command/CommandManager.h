#pragma once

#include <vector>
#include <functional>
#include <string>

#include "../models/ServerCommand.h"
#include "../cloud/ICloudUploader.h"

struct CommandHandlers
{
    std::function<bool(std::string&)> runDiagnostics;
    std::function<bool(std::string&)> testMeter;
    std::function<bool(std::string&)> flushQueue;
};

class CommandManager
{
public:

    explicit CommandManager(
        ICloudUploader& uploader,
        CommandHandlers handlers = {});

    void Process(
        const std::vector<ServerCommand>& commands);

private:

    void ProcessCommand(
        const ServerCommand& command);

private:

    ICloudUploader& m_uploader;
    CommandHandlers m_handlers;
};
