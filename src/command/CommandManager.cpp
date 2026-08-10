#include "CommandManager.h"

#include "../utils/Logger.h"

CommandManager::CommandManager(
    ICloudUploader& uploader)
    : m_uploader(uploader)
{
}

void CommandManager::Process(
    const std::vector<ServerCommand>& commands)
{
    for (const auto& command : commands)
    {
        ProcessCommand(command);
    }
}

void CommandManager::ProcessCommand(
    const ServerCommand& command)
{
    Logger::Info(
        "Executing command : " +
        command.command);

    if (command.command == "SEND_LOGS")
    {
        Logger::Info(
            "SEND_LOGS requested.");

        //
        // TODO
        // Upload current.log
        // Upload previous.log
        //

        Logger::Flush();

Logger::CloseCurrentLog();

bool ok =
    m_uploader.UploadLog(
        "/logs/current.log",
        "current.log");

Logger::ReopenCurrentLog();

if (ok)
{
    Logger::Info(
        "Current log uploaded successfully.");
}
else
{
    Logger::Warning(
        "Current log upload failed.");
}

        m_uploader.UploadLog(
            "/logs/previous.log",
            "previous.log");
    }
    else if (command.command == "CLEAR_LOGS")
    {
        Logger::Info(
            "CLEAR_LOGS requested.");

        // TODO
        // Logger::Clear();
    }
    else if (command.command == "CLEAR_QUEUE")
    {
        Logger::Info(
            "CLEAR_QUEUE requested.");

        // TODO
    }
    else if (command.command == "RESET_SEQUENCE")
    {
        Logger::Info(
            "RESET_SEQUENCE requested.");

        // TODO
    }
    else if (command.command == "CLEAR_STATE")
    {
        Logger::Info(
            "CLEAR_STATE requested.");

        // TODO
    }
    else if (command.command == "SYNC_TIME")
    {
        Logger::Info(
            "SYNC_TIME requested.");
    }
    else if (command.command == "REBOOT")
    {
        Logger::Info(
            "REBOOT requested.");
    }
    else
    {
        Logger::Warning(
            "Unknown command : " +
            command.command);
    }
}