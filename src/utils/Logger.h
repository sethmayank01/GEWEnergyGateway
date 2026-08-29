#pragma once

#include <string>
#include <vector>

#ifdef PLATFORM_ESP32
#include <LittleFS.h>
#else
#include <fstream>
#endif

class Logger
{
public:
    enum class Level { Debug = 0, Info = 1, Warning = 2, Error = 3 };

    static bool Initialize();

    static void Shutdown();

    static void Flush();

    static void CloseCurrentLog();

    static bool ReopenCurrentLog();

    static void DeleteLogs();

    static void SetLevel(Level level);
    static Level GetLevel();

    static void Info(
        const std::string& message);

    static void Warning(
        const std::string& message);

    static void Error(
        const std::string& message);

    static void Hex(
        const std::string& prefix,
        const std::vector<uint8_t>& data);

private:

    static void WriteLog(
        const char* level,
        const std::string& message);

    static void RotateIfRequired();
    
#ifdef PLATFORM_ESP32
    static File m_logFile;
#else
    static std::ofstream m_logFile;
#endif

    static bool m_initialized;
    static uint32_t m_logCounter;
    static Level m_level;
};
