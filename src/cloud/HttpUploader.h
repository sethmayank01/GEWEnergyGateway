#pragma once

#include "ICloudUploader.h"

#include <string>



class HttpUploader :
    public ICloudUploader
{

public:

    explicit HttpUploader(
        const std::string& url);


    bool Upload(
        const std::string& json) override;


private:

    std::string m_url;

#ifdef PLATFORM_WINDOWS
    std::wstring m_host;
    std::wstring m_path;
    uint16_t m_port = 443;
    bool m_secure = true;
#endif

};