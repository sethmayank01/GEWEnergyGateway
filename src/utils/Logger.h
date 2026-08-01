#pragma once

#include <string>
#include <vector>
#include <cstdint>

class Logger
{
public:

    static void Info(const std::string& message);

    static void Warning(const std::string& message);

    static void Error(const std::string& message);

    static void Hex(
        const std::string& prefix,
        const std::vector<uint8_t>& data);
};