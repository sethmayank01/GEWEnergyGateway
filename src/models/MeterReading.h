#pragma once

#include <cstdint>
#include <string>

struct MeterReading
{
    //
    // Voltage - Line to Line
    //
    std::string gatewayId = "";
    std::string apiKey = "";
    std::string device = "";
    std::string firmware = "";
    
    float voltageLLAverage = 0.0f;
    float voltageL12 = 0.0f;
    float voltageL23 = 0.0f;
    float voltageL31 = 0.0f;


    //
    // Voltage - Line to Neutral / Phase
    //
    float voltageLNAverage = 0.0f;
    float voltageL1 = 0.0f;
    float voltageL2 = 0.0f;
    float voltageL3 = 0.0f;



    //
    // Current
    //
    float currentAverage = 0.0f;
    float currentL1 = 0.0f;
    float currentL2 = 0.0f;
    float currentL3 = 0.0f;



    //
    // Frequency
    //
    float frequency = 0.0f;



    //
    // Active Power (W)
    //
    float activePower = 0.0f;
    float activePowerL1 = 0.0f;
    float activePowerL2 = 0.0f;
    float activePowerL3 = 0.0f;



    //
    // Reactive Power (var)
    //
    float reactivePower = 0.0f;
    float reactivePowerL1 = 0.0f;
    float reactivePowerL2 = 0.0f;
    float reactivePowerL3 = 0.0f;



    //
    // Apparent Power (VA)
    //
    float apparentPower = 0.0f;
    float apparentPowerL1 = 0.0f;
    float apparentPowerL2 = 0.0f;
    float apparentPowerL3 = 0.0f;



    //
    // Power Factor
    //
    float powerFactorAverage = 0.0f;
    float powerFactorL1 = 0.0f;
    float powerFactorL2 = 0.0f;
    float powerFactorL3 = 0.0f;



    //
    // Energy
    //
    float energyReceivedWh = 0.0f;
    float energyReceivedVAh = 0.0f;



    //
    // Gateway timestamp
    //
    uint64_t timestamp = 0;
    uint64_t sequence = 0;
};