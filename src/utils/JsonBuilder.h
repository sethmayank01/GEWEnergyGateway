#pragma once

#include "../models/MeterReading.h"

#include <string>


class JsonBuilder
{
public:

    static std::string Build(
        const MeterReading& reading);

};