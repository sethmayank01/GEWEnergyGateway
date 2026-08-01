#pragma once

#include <vector>
#include <cstdint>
#include <string>

class HexDump
{
public:

    static std::string ToString(const std::vector<uint8_t>& data);
};