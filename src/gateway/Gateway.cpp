#include "Gateway.h"

#include "config/Configuration.h"
#include "platform/windows/SerialPortWin.h"
#include "utils/Logger.h"

#include "protocol/ModbusRTU.h"
#include "protocol/ModbusException.h"

#include "devices/ABBM1M12.h"
#include "models/MeterReading.h"

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
    Logger::Info("Meter      : " + cfg.meter.manufacturer + " " + cfg.meter.model);
    Logger::Info("COM Port   : " + cfg.meter.port);
    Logger::Info("Cloud URL  : " + cfg.cloud.url);

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

    ABBM1M12 meter(
        modbus,
        static_cast<uint8_t>(config.Get().meter.slaveId));

    MeterReading reading;

    if (meter.Read(reading))
    {
        Logger::Info("Meter Read Successful.");

        Logger::Info("Voltage L1 : " + std::to_string(reading.voltageL1));
        Logger::Info("Voltage L2 : " + std::to_string(reading.voltageL2));
        Logger::Info("Voltage L3 : " + std::to_string(reading.voltageL3));
               
        
        // Additional parameters will be added later
        // Logger::Info("Current L1 : " + std::to_string(reading.currentL1));
        // Logger::Info("Frequency  : " + std::to_string(reading.frequency));
        // Logger::Info("Power      : " + std::to_string(reading.activePower));
    }
    else
    {
        Logger::Error("Failed to read ABB M1M12.");
    }

    serial.Close();

    Logger::Info("Serial Port Closed.");
}