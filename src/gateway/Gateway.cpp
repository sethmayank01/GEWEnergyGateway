#include "Gateway.h"

#include "config/Configuration.h"
#include "platform/windows/SerialPortWin.h"
#include "utils/Logger.h"

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
        return;

    Logger::Info("Serial communication ready.");

    serial.Close();
}