#pragma once

#include <cstdint>

struct MeterReading
{
    float voltageL1 = 0.0f;
    float voltageL2 = 0.0f;
    float voltageL3 = 0.0f;

    float currentL1 = 0.0f;
    float currentL2 = 0.0f;
    float currentL3 = 0.0f;

    float frequency = 0.0f;

    float activePower = 0.0f;

    float reactivePower = 0.0f;

    float apparentPower = 0.0f;

    float powerFactor = 0.0f;

    uint64_t timestamp = 0;
};