#include "ABBM1M12.h"

#include "ABBRegisters.h"


#include "../utils/ByteBuffer.h"
#include "../protocol/ModbusException.h"
#include "../utils/Logger.h"

#include <vector>

ABBM1M12::ABBM1M12(
    ModbusRTU& modbus,
    uint8_t slaveId)
    :
    m_modbus(modbus),
    m_slave(slaveId)
{
}

bool ABBM1M12::Read(
    MeterReading& reading)
{
    if(!ReadVoltages(reading))
        return false;

    if(!ReadCurrents(reading))
        return false;

    if(!ReadFrequency(reading))
        return false;

    return true;
}

bool ABBM1M12::ReadVoltages(MeterReading& reading)
{
    std::vector<uint8_t> payload;

    ModbusException result =
        m_modbus.ReadHoldingRegisters(
            m_slave,
            ABBRegisters::Voltage::L1,
            6,              // 3 floats = 6 registers = 12 bytes
            payload);

    if (result != ModbusException::None)
    {
        Logger::Error(
            "Voltage Read Failed: " +
            ToString(result));

        return false;
    }

    ByteBuffer buffer(payload);

    reading.voltageL1 =
        buffer.ReadFloat(
            FloatFormat::ByteSwapped,
            0);

    reading.voltageL2 =
        buffer.ReadFloat(
            FloatFormat::ByteSwapped,
            4);

    reading.voltageL3 =
        buffer.ReadFloat(
            FloatFormat::ByteSwapped,
            8);

    return true;
}

bool ABBM1M12::ReadCurrents(MeterReading& reading)
{
    return true;
}

bool ABBM1M12::ReadFrequency(MeterReading& reading)
{
    return true;
}