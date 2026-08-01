#pragma once

#include <string>

struct SerialSettings
{
    std::string port;

    int baud = 9600;

    char parity = 'E';

    int stopBits = 1;

    int dataBits = 8;
};