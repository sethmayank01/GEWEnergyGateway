#include "../WiFiManager.h"

#include "../../utils/Logger.h"

WiFiManager::WiFiManager(
    const GatewayConfig::WiFiCredential wifi[],
    size_t wifiCount)
    :
    m_wifi(wifi),
    m_wifiCount(wifiCount)
{
}

bool WiFiManager::Connect()
{
    Logger::Info(
        "Windows build - WiFi not required.");

    return true;
}

bool WiFiManager::IsConnected() const
{
    return true;
}

void WiFiManager::MaintainConnection()
{
}