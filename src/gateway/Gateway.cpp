#include "Gateway.h"

#include "config/Configuration.h"
#include "platform/windows/SerialPortWin.h"
#include "utils/Logger.h"
#include "protocol/CRC16.h"
#include "protocol/ModbusFrame.h"
#include "protocol/ModbusException.h"
#include "protocol/ModbusRTU.h"


Gateway::Gateway()
{
}

bool Gateway::Initialize()
{
    Logger::Info("Loading configuration...");

    Configuration config;

    if (!config.Load("gateway.json"))
    {
        Logger::Error("Configuration is invalid.");
        return false;
    }

    const auto& cfg = config.Get();

    Logger::Info("Gateway ID : " + cfg.gateway.gatewayId);
    Logger::Info("Meter : " + cfg.meter.manufacturer + " " + cfg.meter.model);
    Logger::Info("COM Port : " + cfg.meter.port);
    Logger::Info("Cloud : " + cfg.cloud.url);

    return true;
}

void Gateway::Run()
{
    Configuration config;

    if (!config.Load("gateway.json"))
    {
        Logger::Error("Configuration error.");
        return;
    }

    SerialPortWin serial(config.Get().meter);

    if (!serial.Open())
    {
        Logger::Error("Cannot open COM port.");
        return;
    }

    
    ModbusRTU modbus(serial);

    std::vector<uint8_t> payload;

    ModbusException result =
        modbus.ReadHoldingRegisters(
            static_cast<uint8_t>(config.Get().meter.slaveId),
            142,
            2,
            payload);

    if (result == ModbusException::None)
    {
        Logger::Info("Modbus Read Successful.");
        Logger::Hex("Payload:", payload);
    }
    else
    {
        Logger::Error("Modbus Read Failed: " + ToString(result));
    }

    serial.Close();

    Logger::Info("Serial Port Closed.");
}