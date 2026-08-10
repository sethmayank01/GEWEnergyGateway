#pragma once

#include "../models/MeterReading.h"
#include "../command/CommandManager.h"
#include "ICloudUploader.h"
#include "../queue/UploadQueue.h"
#include "../health/GatewayHealth.h"
#include <string>

class CloudSyncManager
{
public:

      CloudSyncManager(
        ICloudUploader& uploader,
        GatewayHealth& health);

    bool Upload(
        const MeterReading& reading);

private:

    void RetryPending();
    GatewayHealth& m_health;
    ICloudUploader& m_uploader;
    UploadQueue m_queue;
    CommandManager m_commandManager;
};