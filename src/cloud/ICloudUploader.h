#pragma once

#include <string>


class ICloudUploader
{
public:

    virtual ~ICloudUploader() = default;


    virtual bool Upload(
        const std::string& json)=0;
};