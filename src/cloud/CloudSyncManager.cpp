#include "CloudSyncManager.h"



#include "../utils/JsonBuilder.h"
#include "../utils/Logger.h"

CloudSyncManager::CloudSyncManager(
    ICloudUploader& uploader,
    GatewayHealth& health,
    CommandHandlers handlers)
    :
    m_uploader(uploader),
    m_commandManager(
        uploader,
        [&]() {
            handlers.flushQueue = [this](std::string& result) {
                const size_t before = m_queue.Count();
                const bool ok = RetryPending();
                const size_t after = m_queue.Count();
                result = "Queue flush: " + std::to_string(before) +
                         " pending before, " + std::to_string(after) +
                         " after";
                return ok;
            };
            return handlers;
        }()),
    m_health(health)
{
    m_health.SetPendingUploads(
        m_queue.Count());
}

bool CloudSyncManager::Upload(
    const MeterReading &reading)
{

    RetryPending();
    auto json =
        JsonBuilder::Build(reading);

    Logger::Info("JSON Payload:");
    std::string redactedJson = json;
    const std::string apiKeyMarker = "\"apiKey\":\"";
    const size_t apiKeyStart = redactedJson.find(apiKeyMarker);
    if (apiKeyStart != std::string::npos)
    {
        const size_t valueStart = apiKeyStart + apiKeyMarker.size();
        const size_t valueEnd = redactedJson.find('"', valueStart);
        if (valueEnd != std::string::npos)
            redactedJson.replace(valueStart, valueEnd - valueStart, "[REDACTED]");
    }
    Logger::Info(redactedJson);

    if (m_uploader.Upload(json))
    {
        Logger::Info(
            "Cloud Upload Successful");
            m_commandManager.Process(
        m_uploader.GetCommands());
        m_health.UploadSuccess();
        return true;
    }

    m_health.UploadFailure();

    Logger::Error(
        "Cloud Upload Failed");

    m_queue.Save(
        json,
        reading.timestamp,
        reading.sequence);

    m_health.SetPendingUploads(
    m_queue.Count()); 

    Logger::Info(
        "Reading saved to upload queue.");

    return false;
}

bool CloudSyncManager::RetryPending()
{
    if (!m_queue.HasPending())
        return true;

    Logger::Info(
        "Processing pending upload queue...");
    while (m_queue.HasPending())
    {
        std::string file =
            m_queue.GetOldestFile();

        if (file.empty())
            return false;

        Logger::Info(
            "Retrying queued upload: " + file);

        std::string json =
            m_queue.Load(file);

        if (json.empty())
        {
            Logger::Error(
                "Unable to read queued file.");

            return false;
        }

        if (m_uploader.Upload(json))
        {
            Logger::Info(
                "Queued upload successful.");
            m_health.UploadSuccess();
            m_queue.Remove(file);
            m_health.SetPendingUploads(
    m_queue.Count());
        }
        else
        {
            Logger::Error(
                "Queued upload failed.");

            return false;
        }
    }
    return true;
}

bool CloudSyncManager::Heartbeat(
    const std::string& firmware,
    bool meterConnected)
{
    if (!m_uploader.SendHeartbeat(
            firmware,
            meterConnected,
            m_queue.Count()))
    {
        Logger::Warning("Gateway heartbeat failed.");
        return false;
    }

    Logger::Info("Gateway heartbeat successful.");
    m_commandManager.Process(m_uploader.GetCommands());
    return true;
}
