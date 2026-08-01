#include "Logger.h"
#include "HexDump.h"

#include <iostream>

void Logger::Info(const std::string& message)
{
    std::cout
        << "[INFO] "
        << message
        << std::endl;
}

void Logger::Warning(const std::string& message)
{
    std::cout
        << "[WARNING] "
        << message
        << std::endl;
}

void Logger::Error(const std::string& message)
{
    std::cout
        << "[ERROR] "
        << message
        << std::endl;
}

void Logger::Hex(
    const std::string& prefix,
    const std::vector<uint8_t>& data)
{
    std::cout
        << prefix
        << " "
        << HexDump::ToString(data)
        << std::endl;
}