#pragma once

#include <cstdint>
#include <vector>

class ModbusFrame
{
public:

    static std::vector<uint8_t> ReadHoldingRegisters(
        uint8_t slaveId,
        uint16_t startRegister,
        uint16_t registerCount);

};