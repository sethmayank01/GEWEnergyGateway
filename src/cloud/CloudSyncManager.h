#pragma once

#include "../models/MeterReading.h"
#include "HttpUploader.h"
#include "../queue/UploadQueue.h"

#include <string>

class CloudSyncManager
{
public:

    explicit CloudSyncManager(
        const std::string& url);

    bool Upload(
        const MeterReading& reading);

private:

    void RetryPending();
    
    HttpUploader m_uploader;
    UploadQueue m_queue;
};