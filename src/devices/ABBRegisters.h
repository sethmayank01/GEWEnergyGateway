#pragma once

#include <cstdint>

namespace ABBRegisters
{
    // Voltages
    constexpr uint16_t VoltageL1 = 142;
    constexpr uint16_t VoltageL2 = 144;
    constexpr uint16_t VoltageL3 = 146;

    // Currents
    constexpr uint16_t CurrentL1 = 154;
    constexpr uint16_t CurrentL2 = 156;
    constexpr uint16_t CurrentL3 = 158;

    // Frequency
    constexpr uint16_t Frequency = 172;

    // Power
    constexpr uint16_t ActivePower   = 100;
    constexpr uint16_t ReactivePower = 108;
    constexpr uint16_t ApparentPower = 116;

    // Power Factor
    constexpr uint16_t PowerFactor = 124;

}