#include "GatewayHealth.h"

#include "../utils/Logger.h"
#include "../utils/TimeUtils.h"

void GatewayHealth::MeterReadSuccess()
{
    m_meterConnected = true;

    ++m_successfulReads;

    m_lastReadTimestamp =
        TimeUtils::UnixTimestamp();
    m_lastError.clear();
    m_consecutiveMeterFailures = 0;
}

void GatewayHealth::MeterReadFailure()
{
    m_meterConnected = false;

    ++m_failedReads;

     m_consecutiveMeterFailures++;

    m_lastError =
        "Meter Read Failed";
}

uint32_t GatewayHealth::GetConsecutiveMeterFailures() const
{
    return m_consecutiveMeterFailures;
}

void GatewayHealth::UploadSuccess()
{
    m_cloudConnected = true;

    ++m_successfulUploads;

    m_lastUploadTimestamp =
        TimeUtils::UnixTimestamp();
    m_lastError.clear();
}

void GatewayHealth::UploadFailure()
{
    m_cloudConnected = false;

    ++m_failedUploads;

    m_lastError =
        "Cloud Upload Failed";
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