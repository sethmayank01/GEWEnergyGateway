#include "Gateway.h"

#include "config/Configuration.h"

#include "utils/Logger.h"
#include "utils/TimeUtils.h"

#include "protocol/ModbusRTU.h"

#include "cloud/CloudSyncManager.h"
#include "health/GatewayHealth.h"
#include "health/GatewayState.h"
#include "devices/ABBM1M12.h"
#include "models/MeterReading.h"
#include "cloud/HttpUploader.h"
#include "network/WiFiManager.h"

#include <chrono>
#include <thread>

#ifdef PLATFORM_ESP32
#include "platform/esp32/SerialPortESP32.h"
#include <esp_system.h>
#else
#include "platform/windows/SerialPortWin.h"
#endif

Gateway::Gateway()
{
}

bool Gateway::Initialize()
{
    Logger::Info("Loading configuration...");

    if (!m_config.Load(CONFIG_FILE))
    {
        Logger::Error("Configuration is invalid.");
        return false;
    }

   const auto& cfg = m_config.Get();

    Logger::Info("------------------------------------");
    Logger::Info("GEW Energy Gateway");
    Logger::Info("Version 0.1.0");
    Logger::Info("------------------------------------"); 
    #ifdef PLATFORM_ESP32
    Logger::Info(
    "Reset Reason : " +
    std::to_string(esp_reset_reason()));
    #endif
    Logger::Info("Gateway ID : " + cfg.gateway.gatewayId);
    Logger::Info("API Key    : " + cfg.gateway.apiKey);
    Logger::Info("Firmware   : " + cfg.gateway.firmware);
    Logger::Info("Meter      : " + cfg.meter.manufacturer + " " + cfg.meter.model);
    Logger::Info("COM Port   : " + cfg.meter.port);
    Logger::Info("Cloud URL  : " + cfg.cloud.url);
 #ifdef PLATFORM_ESP32

m_wifi = new WiFiManager(
    cfg.wifi,
    cfg.wifiCount);

if (!m_wifi->Connect())
{
    Logger::Warning(
        "Starting gateway without WiFi.");
}

#endif
    return true;
}

void Gateway::Run()
{
    // Mayank ToDo
    const uint32_t meterFailureLimit = 3;
    const uint32_t recoveryDelaySeconds = 5;

    const auto& cfg = m_config.Get();

    GatewayHealth health;

HttpUploader uploader(
    cfg.cloud.url);

CloudSyncManager cloud(
    uploader,
    health);

   #ifdef PLATFORM_ESP32
    SerialPortESP32 serial(cfg.meter);
#else
    SerialPortWin serial(cfg.meter);
#endif

    if (!serial.Open())
    {
        Logger::Error("Cannot open COM port.");
        return;
    }

    ModbusRTU modbus(serial);
    GatewayState state;

    state.Load(STATE_FILE);

    state.SetGatewayId(
        cfg.gateway.gatewayId);
    ABBM1M12 meter(
        modbus,
        static_cast<uint8_t>(cfg.meter.slaveId));

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
                cfg.gateway.gatewayId;

            reading.device =
                cfg.meter.manufacturer +
                "_" +
                cfg.meter.model;
            reading.apiKey =
                cfg.gateway.apiKey;
            reading.firmware =
                cfg.gateway.firmware;

            reading.sequence =
                state.NextSequence();

            state.Save(
                STATE_FILE);

            reading.timestamp =
                TimeUtils::UnixTimestamp();
            
            #ifdef PLATFORM_ESP32
m_wifi->MaintainConnection();
#endif
            cloud.Upload(reading);
        }
        else
        {
            health.MeterReadFailure();
            Logger::Error("Failed to read meter.");
            Logger::Error(
                "Consecutive failures: " +
                std::to_string(
                    health.GetConsecutiveMeterFailures()));
        }
        if (health.GetConsecutiveMeterFailures() >= meterFailureLimit)
        {
            Logger::Error(
                "Meter communication lost.");
            Logger::Info(
                "Attempting serial recovery...");

            serial.Close();
            Logger::Info(
                "Serial port closed.");

            std::this_thread::sleep_for(
                std::chrono::seconds(
                    recoveryDelaySeconds));
            if (serial.Open())
            {
                Logger::Info(
                    "Serial port recovered.");
                    health.ResetMeterFailureState();
            }
            else
            {
                Logger::Error(
                    "Serial recovery failed.");
            }
        }
        health.PrintStatus();
        std::this_thread::sleep_for(
            std::chrono::seconds(
                cfg.cloud.uploadInterval));
    }
    serial.Close();

    Logger::Info("Serial Port Closed.");
}