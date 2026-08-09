#include "Logger.h"
#include "HexDump.h"

#ifdef PLATFORM_WINDOWS

#include <iostream>

#else

#include <Arduino.h>

#endif

namespace
{
    void WriteLog(
        const char* level,
        const std::string& message)
    {
#ifdef PLATFORM_WINDOWS

        std::cout
            << "["
            << level
            << "] "
            << message
            << std::endl;

#else

        Serial.print("[");

        Serial.print(level);

        Serial.print("] ");

        Serial.println(message.c_str());

#endif
    }
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
        prefix + " " + HexDump::ToString(data));
}