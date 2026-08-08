#include "SerialPortESP32.h"

#include "../../utils/Logger.h"

#include <Arduino.h>

#define METER_TX_PIN 17
#define METER_RX_PIN 18

SerialPortESP32::SerialPortESP32(
    const GatewayConfig::Meter& cfg)
    : m_serial(2),
      m_cfg(cfg)
{
}

bool SerialPortESP32::Open()
{
    uint32_t config = SERIAL_8N1;

    switch (m_cfg.parity)
    {
        case 'E':
            config = (m_cfg.stopBits == 2) ?
                     SERIAL_8E2 : SERIAL_8E1;
            break;

        case 'O':
            config = (m_cfg.stopBits == 2) ?
                     SERIAL_8O2 : SERIAL_8O1;
            break;

        default:
            config = (m_cfg.stopBits == 2) ?
                     SERIAL_8N2 : SERIAL_8N1;
            break;
    }

    m_serial.begin(
        m_cfg.baud,
        config,
        METER_RX_PIN,
        METER_TX_PIN);

    delay(100);

    m_open = true;

    Logger::Info("Serial Port Opened.");

    return true;
}

void SerialPortESP32::Close()
{
    if(m_open)
    {
        m_serial.end();
        m_open = false;
    }
}

void SerialPortESP32::Flush()
{
    while(m_serial.available())
        m_serial.read();
}

bool SerialPortESP32::IsOpen() const
{
    return m_open;
}

bool SerialPortESP32::Write(
    const std::vector<uint8_t>& data)
{
    size_t written =
        m_serial.write(
            data.data(),
            data.size());

    m_serial.flush();

    Logger::Info(
        "Bytes Written: " +
        std::to_string(written));

    return written == data.size();
}

bool SerialPortESP32::Read(
    std::vector<uint8_t>& data,
    size_t bytesToRead,
    uint32_t timeoutMs)
{
    data.clear();

    uint32_t start = millis();

    while(data.size() < bytesToRead)
{
    while(m_serial.available())
    {
        data.push_back(m_serial.read());

        start = millis();      // <-- restart timeout after every byte
    }

    if(millis() - start >= timeoutMs)
        return false;

    delay(1);
}

    return true;
}