#pragma once

#include <cstdint>
#include <cstddef>
#include <string>


class GatewayHealth
{
public:

    GatewayHealth();

    void MeterReadSuccess();

    void MeterReadFailure();


    void UploadSuccess();

    void UploadFailure();


    void SetPendingUploads(
        size_t count);

    void ResetMeterFailureState();
    uint32_t GetConsecutiveMeterFailures() const;


    void PrintStatus() const;


private:
     void InitializeLed();

    void UpdateLed();

    void SetLedColor(
        uint8_t red,
        uint8_t green,
        uint8_t blue);

    bool m_meterConnected = false;

    bool m_cloudConnected = false;


    size_t m_pendingUploads = 0;


    uint64_t m_successfulReads = 0;

    uint64_t m_failedReads = 0;


    uint64_t m_successfulUploads = 0;

    uint64_t m_failedUploads = 0;


    uint64_t m_lastReadTimestamp = 0;

    uint64_t m_lastUploadTimestamp = 0;


    uint32_t m_consecutiveMeterFailures = 0;

   #ifdef PLATFORM_ESP32

    bool m_ledInitialized = false;

#endif
    std::string m_lastError;
};