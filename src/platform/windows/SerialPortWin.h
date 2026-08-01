#pragma once

#include "../../hal/ISerialPort.h"
#include "../../models/GatewayConfig.h"

#include <Windows.h>

class SerialPortWin : public ISerialPort
{
public:

    explicit SerialPortWin(const GatewayConfig::Meter& cfg);

    ~SerialPortWin();

    bool Open() override;

    void Flush() override;
    void Close() override;

    bool IsOpen() const override;

    bool Write(const std::vector<uint8_t>& data) override;

    bool Read(std::vector<uint8_t>& data,
              size_t bytesToRead,
              uint32_t timeoutMs) override;

private:

    HANDLE m_handle;

    GatewayConfig::Meter m_cfg;
};