#include "Logger.h"
#include "HexDump.h"

#ifdef PLATFORM_WINDOWS

#include <iostream>

#else

#include <Arduino.h>

#endif


void Logger::Info(const std::string& message)
{
#ifdef PLATFORM_WINDOWS

    std::cout
        << "[INFO] "
        << message
        << std::endl;

#else

    Serial.print("[INFO] ");
    Serial.println(message.c_str());

#endif
}


void Logger::Warning(const std::string& message)
{
#ifdef PLATFORM_WINDOWS

    std::cout
        << "[WARNING] "
        << message
        << std::endl;

#else

    Serial.print("[WARNING] ");
    Serial.println(message.c_str());

#endif
}


void Logger::Error(const std::string& message)
{
#ifdef PLATFORM_WINDOWS

    std::cout
        << "[ERROR] "
        << message
        << std::endl;

#else

    Serial.print("[ERROR] ");
    Serial.println(message.c_str());

#endif
}


void Logger::Hex(
    const std::string& prefix,
    const std::vector<uint8_t>& data)
{
#ifdef PLATFORM_WINDOWS

    std::cout
        << prefix
        << " "
        << HexDump::ToString(data)
        << std::endl;

#else

    Serial.print(prefix.c_str());
    Serial.print(" ");
    Serial.println(HexDump::ToString(data).c_str());

#endif
}