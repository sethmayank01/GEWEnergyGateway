#include "Logger.h"
#include "HexDump.h"
#include "TimeUtils.h"

#ifdef PLATFORM_WINDOWS

#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

#else

#include <Arduino.h>
#include <LittleFS.h>

#endif

namespace
{
    constexpr size_t MAX_LOG_SIZE = 500 * 1024;

#ifdef PLATFORM_ESP32

    constexpr const char* LOG_FOLDER   = "/logs";
    constexpr const char* CURRENT_LOG  = "/logs/current.log";
    constexpr const char* PREVIOUS_LOG = "/logs/previous.log";

#else

    constexpr const char* LOG_FOLDER   = "logs";
    constexpr const char* CURRENT_LOG  = "logs/current.log";
    constexpr const char* PREVIOUS_LOG = "logs/previous.log";

#endif

    constexpr uint32_t FLUSH_INTERVAL = 20;
    constexpr uint32_t ROTATE_INTERVAL = 100;
}

bool Logger::m_initialized = false;
uint32_t Logger::m_logCounter = 0;

#ifdef PLATFORM_ESP32
File Logger::m_logFile;
#else
std::ofstream Logger::m_logFile;
#endif

bool Logger::Initialize()
{
    if (m_initialized)
        return true;

#ifdef PLATFORM_WINDOWS

    fs::create_directories(LOG_FOLDER);

    if (fs::exists(CURRENT_LOG))
    {
        if (fs::file_size(CURRENT_LOG) >= MAX_LOG_SIZE)
        {
            if (fs::exists(PREVIOUS_LOG))
                fs::remove(PREVIOUS_LOG);

            fs::rename(CURRENT_LOG, PREVIOUS_LOG);
        }
    }

    m_logFile.open(
        CURRENT_LOG,
        std::ios::app);

    if (!m_logFile.is_open())
        return false;

#else

    if (!LittleFS.exists(LOG_FOLDER))
        LittleFS.mkdir(LOG_FOLDER);

    if (LittleFS.exists(CURRENT_LOG))
    {
        File file =
            LittleFS.open(
                CURRENT_LOG,
                FILE_READ);

        if (file)
        {
            if (file.size() >= MAX_LOG_SIZE)
            {
                file.close();

                LittleFS.remove(PREVIOUS_LOG);
                LittleFS.rename(
                    CURRENT_LOG,
                    PREVIOUS_LOG);
            }
            else
            {
                file.close();
            }
        }
    }
   
    m_logFile =
        LittleFS.open(
            CURRENT_LOG,
            FILE_APPEND);

    if (!m_logFile)
        return false;

#endif

    m_initialized = true;

    return true;
}

void Logger::Shutdown()
{
    if (!m_initialized)
        return;

#ifdef PLATFORM_WINDOWS

    m_logFile.close();

#else

    m_logFile.close();

#endif

    m_initialized = false;
}

void Logger::RotateIfRequired()
{
    if (m_logCounter % ROTATE_INTERVAL != 0)
        return;

#ifdef PLATFORM_WINDOWS

    m_logFile.flush();
    m_logFile.close();

    if (fs::file_size(CURRENT_LOG) >= MAX_LOG_SIZE)
    {
        if (fs::exists(PREVIOUS_LOG))
            fs::remove(PREVIOUS_LOG);

        fs::rename(
            CURRENT_LOG,
            PREVIOUS_LOG);
    }

    m_logFile.open(
        CURRENT_LOG,
        std::ios::app);

#else

    m_logFile.flush();
    m_logFile.close();

    File file =
        LittleFS.open(
            CURRENT_LOG,
            FILE_READ);

    if (file)
    {
        if (file.size() >= MAX_LOG_SIZE)
        {
            file.close();

            LittleFS.remove(PREVIOUS_LOG);

            LittleFS.rename(
                CURRENT_LOG,
                PREVIOUS_LOG);
        }
        else
        {
            file.close();
        }
    }

    m_logFile =
        LittleFS.open(
            CURRENT_LOG,
            FILE_APPEND);

#endif
}

void Logger::WriteLog(
    const char* level,
    const std::string& message)
{
    std::string line =
        std::to_string(
            TimeUtils::UnixTimestamp()) +
        " [" +
        level +
        "] " +
        message;

#ifdef PLATFORM_WINDOWS

    std::cout
        << line
        << std::endl;

#else

    Serial.println(
        line.c_str());

#endif

    if (!m_initialized)
        return;

#ifdef PLATFORM_WINDOWS

    m_logFile
        << line
        << std::endl;

    if ((++m_logCounter % FLUSH_INTERVAL) == 0)
        m_logFile.flush();

#else

    m_logFile.println(
        line.c_str());

    if ((++m_logCounter % FLUSH_INTERVAL) == 0)
        m_logFile.flush();

#endif

    RotateIfRequired();
}

void Logger::Flush()
{
    if (!m_initialized)
        return;

#ifdef PLATFORM_WINDOWS

    if (m_logFile.is_open())
    {
        m_logFile.flush();
    }

#else

    if (m_logFile)
    {
        m_logFile.flush();

        //
        // Give LittleFS time to commit
        //
        delay(20);
    }

#endif
}
void Logger::CloseCurrentLog()
{
    if (!m_initialized)
        return;

    Flush();

#ifdef PLATFORM_WINDOWS

    if (m_logFile.is_open())
        m_logFile.close();

#else

    if (m_logFile)
        m_logFile.close();

#endif
}

bool Logger::ReopenCurrentLog()
{
#ifdef PLATFORM_WINDOWS

    m_logFile.open(
        CURRENT_LOG,
        std::ios::app);

    return m_logFile.is_open();

#else

    m_logFile =
        LittleFS.open(
            CURRENT_LOG,
            FILE_APPEND);

    return m_logFile;

#endif
}

void Logger::Info(
    const std::string& message)
{
    WriteLog(
        "INFO",
        message);
}

void Logger::Warning(
    const std::string& message)
{
    WriteLog(
        "WARNING",
        message);
}

void Logger::Error(
    const std::string& message)
{
    WriteLog(
        "ERROR",
        message);
}

void Logger::Hex(
    const std::string& prefix,
    const std::vector<uint8_t>& data)
{
    WriteLog(
        "HEX",
        prefix +
        " " +
        HexDump::ToString(data));
}