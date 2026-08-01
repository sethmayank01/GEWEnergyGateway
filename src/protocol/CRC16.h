#pragma once

#include <vector>
#include <cstdint>

class CRC16
{
public:

    static uint16_t Calculate(const std::vector<uint8_t>& data);

    static bool Verify(const std::vector<uint8_t>& frame);

    static void Append(std::vector<uint8_t>& frame);
};