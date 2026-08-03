#pragma once

#include "../hal/ISerialPort.h"
#include "ModbusException.h"

#include <vector>
#include <cstdint>

class ModbusRTU
{
public:

    explicit ModbusRTU(ISerialPort& serial);

    ModbusException ReadHoldingRegisters(
        uint8_t slave,
        uint16_t address,
        uint16_t registerCount,
        std::vector<uint8_t>& payload);

private:

    ISerialPort& m_serial;
    uint32_t m_interFrameDelayMs = 500;
};