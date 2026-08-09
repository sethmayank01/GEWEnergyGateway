#pragma once

#include "../models/GatewayConfig.h"

class WiFiManager
{
public:

    WiFiManager(
        const GatewayConfig::WiFiCredential wifi[],
        size_t wifiCount);

    bool Connect();

    bool IsConnected() const;

    void MaintainConnection();

private:

    const GatewayConfig::WiFiCredential* m_wifi;

    size_t m_wifiCount;
};