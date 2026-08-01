#include "ModbusFrame.h"
#include "ModbusFunction.h"
#include "CRC16.h"

std::vector<uint8_t> ModbusFrame::ReadHoldingRegisters(
    uint8_t slaveId,
    uint16_t startRegister,
    uint16_t registerCount)
{
    std::vector<uint8_t> frame;

    frame.push_back(slaveId);

    frame.push_back(
    static_cast<uint8_t>(
        ModbusFunction::ReadHoldingRegisters));

    frame.push_back(
        static_cast<uint8_t>((startRegister >> 8) & 0xFF));

    frame.push_back(
        static_cast<uint8_t>(startRegister & 0xFF));

    frame.push_back(
        static_cast<uint8_t>((registerCount >> 8) & 0xFF));

    frame.push_back(
        static_cast<uint8_t>(registerCount & 0xFF));

    CRC16::Append(frame);

    return frame;
}