#pragma once

#include "../models/MeterReading.h"

class IMeter
{
public:

    virtual ~IMeter() = default;

    virtual bool Read(MeterReading& reading)=0;
};