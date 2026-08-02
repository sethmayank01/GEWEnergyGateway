#include "ABBM1M12.h"

#include "ABBRegisters.h"

#include "../utils/FloatDecoder.h"
#include "../protocol/ModbusException.h"
#include "../utils/Logger.h"

#include <vector>

ABBM1M12::ABBM1M12(
    ModbusRTU& modbus,
    uint8_t slaveId)
    :
    m_modbus(modbus),
    m_slaveId(slaveId)
{
}

bool ABBM1M12::Read(MeterReading& reading)
{
    std::vector<uint8_t> payload;

    ModbusException result =
        m_modbus.ReadHoldingRegisters(
            m_slaveId,
            ABBRegisters::VoltageL1,
            2,
            payload);

    if (result != ModbusException::None)
    {
        Logger::Error(
            "ABB Read Failed: " +ToString(result));
           

        return false;
    }

   

        reading.voltageL1 =
    FloatDecoder::Decode(
        payload,
        FloatFormat::BADC);

    return true;
}