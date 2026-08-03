#pragma once

#include "IDevice.h"

#include "../protocol/ModbusRTU.h"

#include <cstdint>

class ABBM1M12 : public IDevice
{
public:

    ABBM1M12(
        ModbusRTU& modbus,
        uint8_t slave);

    bool Read(
        MeterReading& reading) override;

private:

    bool ReadVoltages(
        MeterReading& reading);

    bool ReadCurrents(
        MeterReading& reading);

    bool ReadFrequency(
        MeterReading& reading);

private:

    ModbusRTU& m_modbus;

    uint8_t m_slave;
};