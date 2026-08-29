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
    if(!ReadVoltageBlock(reading))
        return false;

   
   if(!ReadCurrentBlock(reading))
        return false;

        if(!ReadPowerBlock(reading))
        return false;

 

    return true;
}

bool ABBM1M12::ReadVoltageBlock(
    MeterReading& reading)
{
    std::vector<uint8_t> payload;


    ModbusException result =
        m_modbus.ReadHoldingRegisters(
            m_slave,
            ABBRegisters::VoltageBlock::Start,
            ABBRegisters::VoltageBlock::Count,
            payload);


    if (result != ModbusException::None)
    {
        Logger::Error(
            "Voltage Read Failed: " +
            ToString(result));

        return false;
    }


    ByteBuffer buffer(payload);


    reading.voltageLLAverage =
        buffer.ReadFloat(
            FloatFormat::ByteSwapped,
            0);


    reading.voltageL12 =
        buffer.ReadFloat(
            FloatFormat::ByteSwapped,
            4);


    reading.voltageL23 =
        buffer.ReadFloat(
            FloatFormat::ByteSwapped,
            8);


    reading.voltageL31 =
        buffer.ReadFloat(
            FloatFormat::ByteSwapped,
            12);


    reading.voltageLNAverage =
        buffer.ReadFloat(
            FloatFormat::ByteSwapped,
            16);


    reading.voltageL1 =
        buffer.ReadFloat(
            FloatFormat::ByteSwapped,
            20);


    reading.voltageL2 =
        buffer.ReadFloat(
            FloatFormat::ByteSwapped,
            24);


    reading.voltageL3 =
        buffer.ReadFloat(
            FloatFormat::ByteSwapped,
            28);

    return true;
}

bool ABBM1M12::ReadCurrentBlock(
    MeterReading& reading)
{
    std::vector<uint8_t> payload;


    ModbusException result =
        m_modbus.ReadHoldingRegisters(
            m_slave,
            ABBRegisters::CurrentBlock::Start,
            ABBRegisters::CurrentBlock::Count,
            payload);


    if (result != ModbusException::None)
    {
        Logger::Error(
            "Current Block Read Failed: " +
            ToString(result));

        return false;
    }


    ByteBuffer buffer(payload);

   reading.currentAverage =
    buffer.ReadFloat(
        FloatFormat::ByteSwapped,
        0);


reading.currentL1 =
    buffer.ReadFloat(
        FloatFormat::ByteSwapped,
        4);


reading.currentL2 =
    buffer.ReadFloat(
        FloatFormat::ByteSwapped,
        8);


reading.currentL3 =
    buffer.ReadFloat(
        FloatFormat::ByteSwapped,
        12);


reading.frequency =
    buffer.ReadFloat(
        FloatFormat::ByteSwapped,
        16);

reading.energyReceivedWh =
    buffer.ReadFloat(
        FloatFormat::ByteSwapped,
        20);

        reading.energyReceivedVAh =
    buffer.ReadFloat(
        FloatFormat::ByteSwapped,
        24);

    return true;
}

bool ABBM1M12::ReadPowerBlock(
    MeterReading& reading)
{
    std::vector<uint8_t> payload;

    ModbusException result =
        m_modbus.ReadHoldingRegisters(
            m_slave,
            ABBRegisters::PowerBlock::Start,
            ABBRegisters::PowerBlock::Count,
            payload);

    if (result != ModbusException::None)
    {
        Logger::Error(
            "Power Read Failed: " +
            ToString(result));

        return false;
    }

    ByteBuffer buffer(payload);


    // ========================================
    // Active Power
    // 40101 - 40107
    // ========================================

    reading.activePower =
        buffer.ReadFloat(
            FloatFormat::ByteSwapped,
            0);

    reading.activePowerL1 =
        buffer.ReadFloat(
            FloatFormat::ByteSwapped,
            4);

    reading.activePowerL2 =
        buffer.ReadFloat(
            FloatFormat::ByteSwapped,
            8);

    reading.activePowerL3 =
        buffer.ReadFloat(
            FloatFormat::ByteSwapped,
            12);


    // ========================================
    // Reactive Power
    // 40109 - 40115
    // ========================================

    reading.reactivePower =
        buffer.ReadFloat(
            FloatFormat::ByteSwapped,
            16);

    reading.reactivePowerL1 =
        buffer.ReadFloat(
            FloatFormat::ByteSwapped,
            20);

    reading.reactivePowerL2 =
        buffer.ReadFloat(
            FloatFormat::ByteSwapped,
            24);

    reading.reactivePowerL3 =
        buffer.ReadFloat(
            FloatFormat::ByteSwapped,
            28);


    // ========================================
    // Power Factor
    // 40117 - 40123
    // ========================================

    reading.powerFactorAverage =
        buffer.ReadFloat(
            FloatFormat::ByteSwapped,
            32);

    reading.powerFactorL1 =
        buffer.ReadFloat(
            FloatFormat::ByteSwapped,
            36);

    reading.powerFactorL2 =
        buffer.ReadFloat(
            FloatFormat::ByteSwapped,
            40);

    reading.powerFactorL3 =
        buffer.ReadFloat(
            FloatFormat::ByteSwapped,
            44);


    // ========================================
    // Apparent Power
    // 40125 - 40131
    // ========================================

    reading.apparentPower =
        buffer.ReadFloat(
            FloatFormat::ByteSwapped,
            48);

    reading.apparentPowerL1 =
        buffer.ReadFloat(
            FloatFormat::ByteSwapped,
            52);

    reading.apparentPowerL2 =
        buffer.ReadFloat(
            FloatFormat::ByteSwapped,
            56);

    reading.apparentPowerL3 =
        buffer.ReadFloat(
            FloatFormat::ByteSwapped,
            60);


    return true;
}