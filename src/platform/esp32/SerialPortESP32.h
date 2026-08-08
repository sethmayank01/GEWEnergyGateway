#pragma once

#include "../../hal/ISerialPort.h"
#include "../../models/GatewayConfig.h"

#include <HardwareSerial.h>

class SerialPortESP32 : public ISerialPort
{
public:

    explicit SerialPortESP32(
        const GatewayConfig::Meter& cfg);

    bool Open() override;

    void Close() override;

    void Flush() override;

    bool IsOpen() const override;

    bool Write(
        const std::vector<uint8_t>& data) override;

    bool Read(
        std::vector<uint8_t>& data,
        size_t bytesToRead,
        uint32_t timeoutMs) override;

private:

    HardwareSerial m_serial;

    GatewayConfig::Meter m_cfg;

    bool m_open = false;
};