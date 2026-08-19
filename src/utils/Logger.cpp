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

    //
    // Flush at most once every 5 seconds.
    //
    // This avoids constantly writing/committing LittleFS
    // for every few log messages.
    //
    constexpr uint32_t FLUSH_INTERVAL_MS = 5000;

    //
    // Check log file size periodically.
    //
    constexpr uint32_t ROTATE_CHECK_INTERVAL = 50;
}


// ============================================================
// STATIC MEMBERS
// ============================================================

bool Logger::m_initialized = false;

uint32_t Logger::m_logCounter = 0;

#ifdef PLATFORM_ESP32

File Logger::m_logFile;

#else

std::ofstream Logger::m_logFile;

#endif


// ============================================================
// INITIALIZE
// ============================================================

bool Logger::Initialize()
{
    if (m_initialized)
        return true;


#ifdef PLATFORM_WINDOWS

    //
    // Create logs directory
    //
    fs::create_directories(LOG_FOLDER);


    //
    // Check existing log
    //
    if (fs::exists(CURRENT_LOG))
    {
        if (fs::file_size(CURRENT_LOG) >= MAX_LOG_SIZE)
        {
            if (fs::exists(PREVIOUS_LOG))
                fs::remove(PREVIOUS_LOG);

            fs::rename(
                CURRENT_LOG,
                PREVIOUS_LOG);
        }
    }


    //
    // Open current log
    //
    m_logFile.open(
        CURRENT_LOG,
        std::ios::app);


    if (!m_logFile.is_open())
        return false;


#else

    //
    // Create logs directory
    //
    if (!LittleFS.exists(LOG_FOLDER))
    {
        if (!LittleFS.mkdir(LOG_FOLDER))
        {
            Serial.println(
                "[LOGGER] Failed to create log directory.");

            return false;
        }
    }


    //
    // Check current log size
    //
    if (LittleFS.exists(CURRENT_LOG))
    {
        File file =
            LittleFS.open(
                CURRENT_LOG,
                FILE_READ);

        if (file)
        {
            size_t size = file.size();

            file.close();

            if (size >= MAX_LOG_SIZE)
            {
                if (LittleFS.exists(PREVIOUS_LOG))
                    LittleFS.remove(PREVIOUS_LOG);

                LittleFS.rename(
                    CURRENT_LOG,
                    PREVIOUS_LOG);
            }
        }
    }


    //
    // Open current log
    //
    m_logFile =
        LittleFS.open(
            CURRENT_LOG,
            FILE_APPEND);


    if (!m_logFile)
    {
        Serial.println(
            "[LOGGER] Failed to open current.log.");

        return false;
    }

#endif


    m_initialized = true;

    m_logCounter = 0;

    return true;
}


// ============================================================
// SHUTDOWN
// ============================================================

void Logger::Shutdown()
{
    if (!m_initialized)
        return;


#ifdef PLATFORM_WINDOWS

    if (m_logFile.is_open())
        m_logFile.close();

#else

    if (m_logFile)
    {
        m_logFile.flush();
        m_logFile.close();
    }

#endif


    m_initialized = false;
}


// ============================================================
// ROTATE LOG
// ============================================================

void Logger::RotateIfRequired()
{
    //
    // Don't check the filesystem on every message.
    //
    if (m_logCounter % ROTATE_CHECK_INTERVAL != 0)
        return;


#ifdef PLATFORM_WINDOWS

    if (!m_logFile.is_open())
        return;


    m_logFile.flush();

    m_logFile.close();


    //
    // Check file size
    //
    if (
        fs::exists(CURRENT_LOG) &&
        fs::file_size(CURRENT_LOG) >= MAX_LOG_SIZE)
    {
        if (fs::exists(PREVIOUS_LOG))
            fs::remove(PREVIOUS_LOG);

        fs::rename(
            CURRENT_LOG,
            PREVIOUS_LOG);
    }


    //
    // Reopen current log
    //
    m_logFile.open(
        CURRENT_LOG,
        std::ios::app);


#else

    if (!m_logFile)
        return;


    //
    // Check current file size BEFORE closing.
    //
    size_t currentSize =
        m_logFile.size();


    if (currentSize < MAX_LOG_SIZE)
        return;


    //
    // Close current file
    //
    m_logFile.flush();
    m_logFile.close();


    //
    // Remove previous log
    //
    if (LittleFS.exists(PREVIOUS_LOG))
    {
        LittleFS.remove(
            PREVIOUS_LOG);
    }


    //
    // Rename current → previous
    //
    if (LittleFS.exists(CURRENT_LOG))
    {
        LittleFS.rename(
            CURRENT_LOG,
            PREVIOUS_LOG);
    }


    //
    // Create new current log
    //
    m_logFile =
        LittleFS.open(
            CURRENT_LOG,
            FILE_APPEND);


    if (!m_logFile)
    {
        //
        // IMPORTANT:
        // Never crash the gateway because logging failed.
        //
        Serial.println(
            "[LOGGER] WARNING: "
            "Unable to reopen current.log.");
    }

#endif
}


// ============================================================
// WRITE LOG
// ============================================================

void Logger::WriteLog(
    const char* level,
    const std::string& message)
{
    //
    // Build log line.
    //
    std::string line =
        std::to_string(
            TimeUtils::UnixTimestamp()) +
        " [" +
        level +
        "] " +
        message;


    //
    // ALWAYS write to Serial first.
    //
    // Serial is our primary diagnostic path and should
    // continue working even if LittleFS fails.
    //
#ifdef PLATFORM_WINDOWS

    std::cout
        << line
        << std::endl;

#else

    Serial.println(
        line.c_str());

#endif


    //
    // If filesystem logging is unavailable,
    // simply continue.
    //
    if (!m_initialized)
        return;


#ifdef PLATFORM_WINDOWS

    if (!m_logFile.is_open())
        return;


    //
    // Write line
    //
    m_logFile
        << line
        << std::endl;


    ++m_logCounter;


    //
    // Rotate based on actual file size.
    //
    RotateIfRequired();


#else

    //
    // Check that file is still valid.
    //
    if (!m_logFile)
    {
        //
        // Try to recover the log file.
        //
        m_logFile =
            LittleFS.open(
                CURRENT_LOG,
                FILE_APPEND);

        if (!m_logFile)
        {
            //
            // Logging failure must NOT stop gateway.
            //
            return;
        }
    }


    //
    // Write line
    //
    m_logFile.println(
        line.c_str());


    ++m_logCounter;


    //
    // Periodic flush.
    //
    //
    // IMPORTANT:
    // We intentionally do NOT call flush() every
    // 20 messages anymore.
    //
    static uint32_t lastFlush = 0;

    uint32_t now = millis();


    if (
        lastFlush == 0 ||
        (now - lastFlush) >= FLUSH_INTERVAL_MS)
    {
        //
        // Flush only if file is valid.
        //
        if (m_logFile)
        {
            m_logFile.flush();
        }

        lastFlush = now;
    }


    //
    // Check for log rotation.
    //
    RotateIfRequired();

#endif
}


// ============================================================
// FLUSH
// ============================================================

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
        // Give LittleFS time to commit.
        //
        delay(20);
    }

#endif
}


// ============================================================
// CLOSE CURRENT LOG
// ============================================================

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


// ============================================================
// REOPEN CURRENT LOG
// ============================================================

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


// ============================================================
// INFO
// ============================================================

void Logger::Info(
    const std::string& message)
{
    WriteLog(
        "INFO",
        message);
}


// ============================================================
// WARNING
// ============================================================

void Logger::Warning(
    const std::string& message)
{
    WriteLog(
        "WARNING",
        message);
}


// ============================================================
// ERROR
// ============================================================

void Logger::Error(
    const std::string& message)
{
    WriteLog(
        "ERROR",
        message);
}


// ============================================================
// HEX
// ============================================================

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

void Logger::DeleteLogs()
{
#ifdef PLATFORM_ESP32

    if (LittleFS.exists(CURRENT_LOG))
    {
        LittleFS.remove(CURRENT_LOG);
    }

    if (LittleFS.exists(PREVIOUS_LOG))
    {
        LittleFS.remove(PREVIOUS_LOG);
    }

#endif
}