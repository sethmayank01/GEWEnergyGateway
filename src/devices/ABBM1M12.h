#pragma once

#include "IMeter.h"

#include "../protocol/ModbusRTU.h"

#include <cstdint>

class ABBM1M12 : public IMeter
{
public:

    ABBM1M12(
        ModbusRTU& modbus,
        uint8_t slaveId);

    bool Read(MeterReading& reading) override;

private:

    ModbusRTU& m_modbus;

    uint8_t m_slaveId;
};