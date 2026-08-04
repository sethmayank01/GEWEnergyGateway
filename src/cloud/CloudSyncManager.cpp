#include "CloudSyncManager.h"

#include "HttpUploader.h"

#include "../utils/JsonBuilder.h"
#include "../utils/Logger.h"

CloudSyncManager::CloudSyncManager(
    const std::string &url)
    : m_uploader(url)
{
}

bool CloudSyncManager::Upload(
    const MeterReading &reading)
{

    RetryPending();
    auto json =
        JsonBuilder::Build(reading);

    Logger::Info("JSON Payload:");
    Logger::Info(json);

    if (m_uploader.Upload(json))
    {
        Logger::Info(
            "Cloud Upload Successful");

        return true;
    }

    Logger::Error(
        "Cloud Upload Failed");

    m_queue.Save(
        json,
        reading.timestamp,
        reading.sequence);

    Logger::Info(
        "Reading saved to upload queue.");

    return false;
}

void CloudSyncManager::RetryPending()
{
    if (!m_queue.HasPending())
        return;

    Logger::Info(
        "Processing pending upload queue...");
    while (m_queue.HasPending())
    {
        std::string file =
            m_queue.GetOldestFile();

        if (file.empty())
            return;

        Logger::Info(
            "Retrying queued upload: " + file);

        std::string json =
            m_queue.Load(file);

        if (json.empty())
        {
            Logger::Error(
                "Unable to read queued file.");

            return;
        }

        if (m_uploader.Upload(json))
        {
            Logger::Info(
                "Queued upload successful.");

            m_queue.Remove(file);
        }
        else
        {
            Logger::Error(
                "Queued upload failed.");

            return;
        }
    }
}