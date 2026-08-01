#pragma once

#include <string>

struct GatewayConfig
{
    struct Gateway
    {
        std::string gatewayId;
        std::string firmware;
        std::string hardware;
    } gateway;

    struct Meter
    {
        std::string manufacturer;
        std::string model;

        std::string port;

        int baud = 9600;

        char parity = 'E';

        int stopBits = 1;

        int slaveId = 1;
    } meter;

    struct Cloud
    {
        std::string url;

        int uploadInterval = 2;

        std::string apiKey;
    } cloud;
};