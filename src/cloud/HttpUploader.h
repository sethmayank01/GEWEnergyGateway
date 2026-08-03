#pragma once

#include "ICloudUploader.h"

#include <string>


class HttpUploader :
    public ICloudUploader
{

public:

    HttpUploader(
        const std::string& url);


    bool Upload(
        const std::string& json) override;


private:

    std::string m_url;

};