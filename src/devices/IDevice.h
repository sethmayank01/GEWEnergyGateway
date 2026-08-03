#pragma once

#include "../models/MeterReading.h"

class IDevice
{
public:

    virtual ~IDevice() = default;

    virtual bool Read(MeterReading& reading)=0;
};