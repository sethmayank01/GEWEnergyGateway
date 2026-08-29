#pragma once

#include <string>

#ifdef PLATFORM_WINDOWS

constexpr const char* CONFIG_FILE = "data/gateway.json";
constexpr const char* STATE_FILE  = "data/gateway_state.json";
constexpr const char* QUEUE_DIR    = "queue";

#else

constexpr const char* CONFIG_FILE = "/gateway.json";
constexpr const char* STATE_FILE  = "/gateway_state.json";
constexpr const char* QUEUE_DIR    = "/queue";

#endif

constexpr int MAX_WIFI_NETWORKS = 8;
struct GatewayConfig
{
    struct Gateway
    {
        std::string gatewayId;
        std::string apiKey;
        std::string firmware;
        std::string hardware;
        uint64_t lastSequence = 0;
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

        
    } cloud;

    struct WiFiCredential
    {
        std::string ssid;
        std::string password;
    };

    WiFiCredential wifi[MAX_WIFI_NETWORKS];

    int wifiCount = 0;
    
};
