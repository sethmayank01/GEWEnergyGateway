#pragma once

#include "../models/MeterReading.h"
#include "HttpUploader.h"
#include "../queue/UploadQueue.h"
#include "../health/GatewayHealth.h"
#include <string>

class CloudSyncManager
{
public:

     CloudSyncManager(
        const std::string& url,
        GatewayHealth& health);

    bool Upload(
        const MeterReading& reading);

private:

    void RetryPending();
    GatewayHealth& m_health;
    HttpUploader m_uploader;
    UploadQueue m_queue;
};