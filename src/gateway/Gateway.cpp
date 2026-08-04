#include "Gateway.h"

#include "config/Configuration.h"
#include "platform/windows/SerialPortWin.h"

#include "utils/Logger.h"
#include "utils/TimeUtils.h"

#include "protocol/ModbusRTU.h"

#include "cloud/CloudSyncManager.h"
#include "health/GatewayHealth.h"
#include "health/GatewayState.h"
#include "devices/ABBM1M12.h"
#include "models/MeterReading.h"

#include <chrono>
#include <thread>

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

    const auto &cfg = config.Get();

    Logger::Info("Gateway ID : " + cfg.gateway.gatewayId);
    Logger::Info("Firmware   : " + cfg.gateway.firmware);
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
    GatewayHealth health;
    CloudSyncManager cloud(
    config.Get().cloud.url,
    health);

    SerialPortWin serial(config.Get().meter);
    
    if (!serial.Open())
    {
        Logger::Error("Cannot open COM port.");
        return;
    }

    ModbusRTU modbus(serial);
    GatewayState state;

state.Load(
    "data/gateway_state.json");

state.SetGatewayId(
    config.Get().gateway.gatewayId);
    ABBM1M12 meter(
        modbus,
        static_cast<uint8_t>(config.Get().meter.slaveId));

    while (true)
    {
        MeterReading reading;

        if (meter.Read(reading))
        {
            health.MeterReadSuccess();
            Logger::Info("--------------------------------");
            Logger::Info("ABB M1M12 Measurement Snapshot");
            Logger::Info("--------------------------------");

            //
            // Voltage
            //
            Logger::Info(
                "Voltage L1 : " +
                std::to_string(reading.voltageL1));

            Logger::Info(
                "Voltage L2 : " +
                std::to_string(reading.voltageL2));

            Logger::Info(
                "Voltage L3 : " +
                std::to_string(reading.voltageL3));

            Logger::Info(
                "Voltage L12 : " +
                std::to_string(reading.voltageL12));

            Logger::Info(
                "Voltage L23 : " +
                std::to_string(reading.voltageL23));

            Logger::Info(
                "Voltage L31 : " +
                std::to_string(reading.voltageL31));

            //
            // Current
            //
            Logger::Info(
                "Current L1 : " +
                std::to_string(reading.currentL1));

            Logger::Info(
                "Current L2 : " +
                std::to_string(reading.currentL2));

            Logger::Info(
                "Current L3 : " +
                std::to_string(reading.currentL3));

            Logger::Info(
                "Current Average : " +
                std::to_string(reading.currentAverage));

            //
            // System
            //
            Logger::Info(
                "Frequency : " +
                std::to_string(reading.frequency));

            //
            // Power
            //
            Logger::Info(
                "Active Power : " +
                std::to_string(reading.activePower));

            Logger::Info(
                "Reactive Power : " +
                std::to_string(reading.reactivePower));

            Logger::Info(
                "Apparent Power : " +
                std::to_string(reading.apparentPower));

            //
            // Power Factor
            //
            Logger::Info(
                "Power Factor Average : " +
                std::to_string(reading.powerFactorAverage));

            Logger::Info(
                "Power Factor L1 : " +
                std::to_string(reading.powerFactorL1));

            Logger::Info(
                "Power Factor L2 : " +
                std::to_string(reading.powerFactorL2));

            Logger::Info(
                "Power Factor L3 : " +
                std::to_string(reading.powerFactorL3));

            //
            // Energy
            //
            Logger::Info(
                "Wh Received : " +
                std::to_string(reading.energyReceivedWh));

            reading.gatewayId =
                config.Get().gateway.gatewayId;

            reading.device =
                config.Get().meter.manufacturer +
                "_" +
                config.Get().meter.model;

            reading.firmware =
                config.Get().gateway.firmware;

            reading.sequence =
    state.NextSequence();

    state.Save(
    "data/gateway_state.json");

            reading.timestamp =
                TimeUtils::UnixTimestamp();

            cloud.Upload(reading);
        }
        else
        {
            health.MeterReadFailure();
            Logger::Error("Failed to read meter.");
        }
        health.PrintStatus();
        std::this_thread::sleep_for(
            std::chrono::seconds(
                config.Get().cloud.uploadInterval));
    }
    serial.Close();

    Logger::Info("Serial Port Closed.");
}