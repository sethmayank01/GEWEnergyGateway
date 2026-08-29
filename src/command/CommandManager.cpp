#include "CommandManager.h"

#include "../utils/Logger.h"

#include <algorithm>
#include <cctype>
#include <utility>

#ifdef PLATFORM_ESP32
#include <ArduinoJson.h>
#else
#include "../../thirdparty/nlohmann/json.hpp"
#endif

#ifdef PLATFORM_ESP32
#include <Arduino.h>
#include <Preferences.h>
#endif

namespace
{
#ifdef PLATFORM_ESP32
    constexpr const char* COMMAND_NVS_NAMESPACE = "gew_commands";
    constexpr const char* LAST_COMMAND_ID_KEY = "last_id";
    constexpr const char* LAST_COMMAND_OK_KEY = "last_ok";

    bool LoadCompletedCommand(uint32_t& commandId, bool& succeeded)
    {
        Preferences state;
        // Open read/write so the namespace is created quietly on first use.
        if (!state.begin(COMMAND_NVS_NAMESPACE, false))
            return false;
        commandId = state.getUInt(LAST_COMMAND_ID_KEY, 0);
        succeeded = state.getBool(LAST_COMMAND_OK_KEY, false);
        state.end();
        return commandId != 0;
    }

    void SaveCompletedCommand(uint32_t commandId, bool succeeded)
    {
        Preferences state;
        if (!state.begin(COMMAND_NVS_NAMESPACE, false))
            return;
        state.putUInt(LAST_COMMAND_ID_KEY, commandId);
        state.putBool(LAST_COMMAND_OK_KEY, succeeded);
        state.end();
    }
#endif
}

CommandManager::CommandManager(
    ICloudUploader& uploader,
    CommandHandlers handlers)
    : m_uploader(uploader),
      m_handlers(std::move(handlers))
{
}

void CommandManager::Process(const std::vector<ServerCommand>& commands)
{
    for (const auto& command : commands)
        ProcessCommand(command);
}

void CommandManager::ProcessCommand(const ServerCommand& command)
{
    if (command.id == 0 || command.command.empty())
    {
        Logger::Warning("Ignoring command with missing ID or name.");
        return;
    }

#ifdef PLATFORM_ESP32
    uint32_t lastCommandId = 0;
    bool lastCommandSucceeded = false;
    if (LoadCompletedCommand(lastCommandId, lastCommandSucceeded) &&
        command.id <= lastCommandId)
    {
        Logger::Warning("Command ID " + std::to_string(command.id) +
                        " was already processed; not executing it again.");
        m_uploader.ReportCommandStatus(
            command.id,
            lastCommandSucceeded ? "SUCCESS" : "FAILED",
            "Previously processed by gateway");
        return;
    }
#endif

    Logger::Info("Executing command ID " + std::to_string(command.id) +
                 " : " + command.command);
    m_uploader.ReportCommandStatus(command.id, "RUNNING", "Command started");

    bool succeeded = false;
    bool restartAfterSuccess = false;
    std::string resultMessage;

    if (command.command == "SEND_LOGS")
    {
        Logger::Info("SEND_LOGS requested.");
        Logger::Flush();
        Logger::CloseCurrentLog();
        succeeded = m_uploader.UploadLog("/logs/current.log", "current.log");

        if (!Logger::ReopenCurrentLog())
            Logger::Warning("Unable to reopen current log after upload.");

        if (succeeded)
        {
            // The previous log is optional; absence/failure does not invalidate
            // successful delivery of the current diagnostic log.
            m_uploader.UploadLog("/logs/previous.log", "previous.log");
            resultMessage = "Current gateway log uploaded";
            Logger::Info("Current log uploaded successfully.");
        }
        else
        {
            resultMessage = "Current gateway log upload failed";
            Logger::Warning("Current log upload failed.");
        }
    }
    else if (command.command == "UPDATE_FIRMWARE")
    {
        Logger::Info("UPDATE_FIRMWARE requested.");
        std::string installedVersion;
        succeeded = m_uploader.InstallFirmware(
            command.id,
            installedVersion,
            resultMessage);
        restartAfterSuccess = succeeded;
        if (succeeded)
            Logger::Info(resultMessage);
        else
            Logger::Error(resultMessage);
    }
    else if (command.command == "UPDATE_CONFIG")
    {
        Logger::Info("UPDATE_CONFIG requested.");
        succeeded = m_uploader.RefreshConfiguration(command.id, resultMessage);
        restartAfterSuccess = succeeded;
        if (succeeded)
            Logger::Info(resultMessage);
        else
            Logger::Error(resultMessage);
    }
    else if (command.command == "RUN_DIAGNOSTICS")
    {
        Logger::Info("RUN_DIAGNOSTICS requested.");
        succeeded = m_handlers.runDiagnostics &&
                    m_handlers.runDiagnostics(resultMessage);
        if (!m_handlers.runDiagnostics)
            resultMessage = "Diagnostics handler is unavailable";
    }
    else if (command.command == "TEST_METER")
    {
        Logger::Info("TEST_METER requested.");
        succeeded = m_handlers.testMeter &&
                    m_handlers.testMeter(resultMessage);
        if (!m_handlers.testMeter)
            resultMessage = "Meter test handler is unavailable";
    }
    else if (command.command == "FLUSH_QUEUE")
    {
        Logger::Info("FLUSH_QUEUE requested.");
        succeeded = m_handlers.flushQueue &&
                    m_handlers.flushQueue(resultMessage);
        if (!m_handlers.flushQueue)
            resultMessage = "Queue handler is unavailable";
    }
    else if (command.command == "SET_LOG_LEVEL")
    {
        std::string level;
        try
        {
#ifdef PLATFORM_ESP32
            JsonDocument parameters;
            if (!deserializeJson(parameters, command.parameters))
                level = std::string(parameters["level"] | "");
#else
            const auto parameters = nlohmann::json::parse(command.parameters);
            level = parameters.value("level", "");
#endif
        }
        catch (...)
        {
            level.clear();
        }

        std::transform(
            level.begin(), level.end(), level.begin(),
            [](unsigned char character) {
                return static_cast<char>(std::toupper(character));
            });
        if (level == "DEBUG")
            Logger::SetLevel(Logger::Level::Debug);
        else if (level == "INFO")
            Logger::SetLevel(Logger::Level::Info);
        else if (level == "WARNING")
            Logger::SetLevel(Logger::Level::Warning);
        else if (level == "ERROR")
            Logger::SetLevel(Logger::Level::Error);
        else
        {
            resultMessage = "Invalid log level; use DEBUG, INFO, WARNING or ERROR";
        }

        succeeded = !level.empty() &&
                    (level == "DEBUG" || level == "INFO" ||
                     level == "WARNING" || level == "ERROR");
        if (succeeded)
            resultMessage = "Log level set to " + level;
    }
    else if (command.command == "RESTART_GATEWAY")
    {
        Logger::Info("RESTART_GATEWAY requested.");
        succeeded = true;
        restartAfterSuccess = true;
        resultMessage = "Gateway restarting";
    }
    else
    {
        resultMessage = "Unsupported command: " + command.command;
        Logger::Warning(resultMessage);
    }

#ifdef PLATFORM_ESP32
    // Persist before the terminal acknowledgement. A redelivered command is
    // acknowledged but never executes twice if that HTTP request is lost.
    SaveCompletedCommand(command.id, succeeded);
#endif

    m_uploader.ReportCommandStatus(
        command.id,
        succeeded ? "SUCCESS" : "FAILED",
        resultMessage);

#ifdef PLATFORM_ESP32
    if (restartAfterSuccess)
    {
        Logger::Flush();
        delay(1000);
        ESP.restart();
    }
#endif
}
