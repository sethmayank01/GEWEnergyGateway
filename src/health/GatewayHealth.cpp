#include "GatewayHealth.h"
#ifdef PLATFORM_ESP32
#include <Adafruit_NeoPixel.h>
#endif
#include "../utils/Logger.h"
#include "../utils/TimeUtils.h"

GatewayHealth::GatewayHealth()
{
#ifdef PLATFORM_ESP32
    InitializeLed();
#endif
}

#ifdef PLATFORM_ESP32


namespace
{
   constexpr uint8_t LED_PIN = 21;

Adafruit_NeoPixel led(
    1,
    LED_PIN,
    NEO_RGB + NEO_KHZ800);
}

void GatewayHealth::InitializeLed()
{
    led.begin();
    led.clear();
    led.show();

    m_ledInitialized = true;

    SetLedColor(255, 0, 0);     // Booting
}

void GatewayHealth::SetLedColor(
    uint8_t r,
    uint8_t g,
    uint8_t b)
{
    if (!m_ledInitialized)
        return;

    led.setPixelColor(
        0,
        led.Color(r, g, b));

    led.show();
}

#endif

void GatewayHealth::UpdateLed()
{
#ifdef PLATFORM_ESP32

    if (!m_cloudConnected)
    {
        SetLedColor(255, 0, 255);      // Purple
    }
    else if (!m_meterConnected)
    {
        SetLedColor(255, 255, 0);      // Yellow
    }
    else if (m_pendingUploads > 0)
    {
        SetLedColor(255, 120, 0);      // Orange
    }
    else
    {
        SetLedColor(0, 255, 0);        // Green
    }

#endif
}

void GatewayHealth::MeterReadSuccess()
{
    m_meterConnected = true;

    ++m_successfulReads;

    m_lastReadTimestamp =
        TimeUtils::UnixTimestamp();
    m_lastError.clear();
    m_consecutiveMeterFailures = 0;
    UpdateLed();
}

void GatewayHealth::MeterReadFailure()
{
    m_meterConnected = false;

    ++m_failedReads;

     m_consecutiveMeterFailures++;

    m_lastError =
        "Meter Read Failed";
        UpdateLed();
}

uint32_t GatewayHealth::GetConsecutiveMeterFailures() const
{
    return m_consecutiveMeterFailures;
}

void GatewayHealth::ResetMeterFailureState()
{
    m_consecutiveMeterFailures = 0;
}

void GatewayHealth::UploadSuccess()
{
    m_cloudConnected = true;

    ++m_successfulUploads;

    m_lastUploadTimestamp =
        TimeUtils::UnixTimestamp();
    m_lastError.clear();
    UpdateLed();
}

void GatewayHealth::UploadFailure()
{
    m_cloudConnected = false;

    ++m_failedUploads;

    m_lastError =
        "Cloud Upload Failed";
        UpdateLed();
}

void GatewayHealth::SetPendingUploads(
    size_t count)
{
    m_pendingUploads = count;
}

void GatewayHealth::PrintStatus() const
{
    Logger::Info("==================================");
    Logger::Info("Gateway Health");
    Logger::Info("==================================");

    Logger::Info(
        std::string("Meter  : ") +
        (m_meterConnected ? "Connected"
                          : "Disconnected"));

    Logger::Info(
        std::string("Cloud  : ") +
        (m_cloudConnected ? "Connected"
                          : "Disconnected"));

    Logger::Info(
        "Pending Uploads : " +
        std::to_string(m_pendingUploads));

    Logger::Info(
        "Reads : " +
        std::to_string(m_successfulReads) +
        " OK / " +
        std::to_string(m_failedReads) +
        " Failed");

    Logger::Info(
        "Uploads : " +
        std::to_string(m_successfulUploads) +
        " OK / " +
        std::to_string(m_failedUploads) +
        " Failed");

    if (!m_lastError.empty())
    {
        Logger::Info(
            "Last Error : " +
            m_lastError);
    }

    Logger::Info("==================================");
   
}