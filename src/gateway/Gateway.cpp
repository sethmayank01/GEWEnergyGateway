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
#include <WiFi.h>
#include <esp_system.h>
#include <esp_ota_ops.h>
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
    // API keys are credentials and must never be written to serial or log files.
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
    cfg.cloud.url,
    cfg.gateway.gatewayId,
    cfg.gateway.apiKey);

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

    if (state.GetSequence() < cfg.gateway.lastSequence)
    {
        Logger::Warning(
            "Restoring sequence from server configuration: " +
            std::to_string(cfg.gateway.lastSequence));
        state.SetSequence(cfg.gateway.lastSequence);
        state.Save(STATE_FILE);
    }
    ABBM1M12 meter(
        modbus,
        static_cast<uint8_t>(cfg.meter.slaveId));

    CommandHandlers commandHandlers;
    commandHandlers.runDiagnostics = [&](std::string& result) {
        Logger::Info("========== Remote Diagnostics ==========");
        health.PrintStatus();
#ifdef PLATFORM_ESP32
        Logger::Info("WiFi RSSI : " + std::to_string(WiFi.RSSI()) + " dBm");
        Logger::Info("Free Heap : " + std::to_string(ESP.getFreeHeap()));
        Logger::Info("Uptime ms : " + std::to_string(millis()));
        Logger::Info("Reset Reason : " + std::to_string(esp_reset_reason()));
        result = "Diagnostics recorded; RSSI " + std::to_string(WiFi.RSSI()) +
                 " dBm, free heap " + std::to_string(ESP.getFreeHeap());
#else
        result = "Diagnostics recorded in gateway log";
#endif
        Logger::Info("========================================");
        return true;
    };
    commandHandlers.testMeter = [&](std::string& result) {
        MeterReading diagnosticReading;
        if (!meter.Read(diagnosticReading))
        {
            result = "Meter test failed";
            return false;
        }
        result = "Meter test passed; L1 " +
                 std::to_string(diagnosticReading.voltageL1) +
                 " V, frequency " +
                 std::to_string(diagnosticReading.frequency) + " Hz";
        return true;
    };

    CloudSyncManager cloud(
        uploader,
        health,
        commandHandlers);

#ifdef PLATFORM_ESP32
    constexpr auto heartbeatInterval = std::chrono::seconds(30);
    auto lastHeartbeat = std::chrono::steady_clock::now() - heartbeatInterval;
    bool meterConnected = false;
    bool otaImageValidated = false;

    auto sendHeartbeatIfDue = [&]() {
        const auto now = std::chrono::steady_clock::now();
        if (now - lastHeartbeat < heartbeatInterval)
            return;

        m_wifi->MaintainConnection();
        if (m_wifi->IsConnected() &&
            cloud.Heartbeat(cfg.gateway.firmware, meterConnected) &&
            !otaImageValidated)
        {
            const esp_err_t validation =
                esp_ota_mark_app_valid_cancel_rollback();
            if (validation == ESP_OK || validation == ESP_ERR_NOT_FOUND)
            {
                otaImageValidated = true;
                Logger::Info("Running firmware validated by secure heartbeat.");
            }
            else
            {
                Logger::Warning("Unable to mark OTA firmware valid.");
            }
        }
        lastHeartbeat = now;
    };
#endif

    while (true)
    {
#ifdef PLATFORM_ESP32
        sendHeartbeatIfDue();
#endif
        MeterReading reading;

        if (meter.Read(reading))
        {
#ifdef PLATFORM_ESP32
            meterConnected = true;
#endif
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

            Logger::Info(
                "VAh Received : " +
                std::to_string(reading.energyReceivedVAh));


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
#ifdef PLATFORM_ESP32
            meterConnected = false;
#endif
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

        // Keep Wi-Fi recovery and the command heartbeat alive while waiting
        // for the next meter cycle. A single long sleep would make commands
        // unreachable whenever the configured upload interval is large.
        const auto nextReadingAt =
            std::chrono::steady_clock::now() +
            std::chrono::seconds(cfg.cloud.uploadInterval);
        while (std::chrono::steady_clock::now() < nextReadingAt)
        {
#ifdef PLATFORM_ESP32
            sendHeartbeatIfDue();
#endif
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
    serial.Close();

    Logger::Info("Serial Port Closed.");
}
