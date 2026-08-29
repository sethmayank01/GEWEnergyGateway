#include "ModbusRTU.h"

#include "ModbusFrame.h"
#include "ModbusFunction.h"
#include "CRC16.h"

#include "utils/Logger.h"
#include <thread>
#include <chrono>

ModbusRTU::ModbusRTU(
        ISerialPort& serial)
    :
      m_serial(serial)
{

}

ModbusException
ModbusRTU::ReadHoldingRegisters(

        uint8_t slave,

        uint16_t address,

        uint16_t registerCount,

        std::vector<uint8_t>& payload)
{
    auto frame =
        ModbusFrame::ReadHoldingRegisters(
            slave,
            address,
            registerCount);

              
    m_serial.Flush();
    Logger::Hex("MODBUS TX:", frame);
    if(!m_serial.Write(frame))
        return ModbusException::CommunicationError;

    

std::vector<uint8_t> header;

if (!m_serial.Read(header, 3, 1000))
{
    return ModbusException::Timeout;
}

Logger::Hex("RX Header:", header);

if (header.size() != 3)
{
    return ModbusException::CommunicationError;
}

if (header[0] != slave)
{
    return ModbusException::InvalidSlave;
}

uint8_t function =
    static_cast<uint8_t>(
        ModbusFunction::ReadHoldingRegisters);

if (header[1] == (function | 0x80))
{
    return ModbusException::ExceptionResponse;
}

if (header[1] != function)
{
    return ModbusException::InvalidFunction;
}

uint8_t byteCount = header[2];

std::vector<uint8_t> body;

if (!m_serial.Read(body, byteCount + 2, 1000))
{
    return ModbusException::Timeout;
}

Logger::Hex("RX Body:", body);

std::vector<uint8_t> response;

response.insert(
    response.end(),
    header.begin(),
    header.end());

response.insert(
    response.end(),
    body.begin(),
    body.end());

Logger::Hex("RX:", response);

if (!CRC16::Verify(response))
{
    return ModbusException::CRCError;
}

payload.assign(
    body.begin(),
    body.begin() + byteCount);

 // ABB M1M12 requires inter-request delay.
// Without this delay, consecutive Modbus transactions may timeout.
#ifdef PLATFORM_ESP32
// Mayank delay(500);
#else
std::this_thread::sleep_for(
    std::chrono::milliseconds(500));
#endif
return ModbusException::None;
    
}
