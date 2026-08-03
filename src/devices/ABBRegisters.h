#pragma once

#include <cstdint>

namespace ABBRegisters
{
    namespace Voltage
    {
        constexpr uint16_t L1 = 142;
        constexpr uint16_t L2 = 144;
        constexpr uint16_t L3 = 146;
    }

    namespace Current
    {
        constexpr uint16_t L1 = 154;
        constexpr uint16_t L2 = 156;
        constexpr uint16_t L3 = 158;
    }

    namespace System
    {
        constexpr uint16_t Frequency = 172;

        constexpr uint16_t PowerFactor = 124;
    }

    namespace Power
    {
        constexpr uint16_t Active = 100;

        constexpr uint16_t Reactive = 108;

        constexpr uint16_t Apparent = 116;
    }

    namespace Energy
    {
        constexpr uint16_t Import = 200;
    }
}