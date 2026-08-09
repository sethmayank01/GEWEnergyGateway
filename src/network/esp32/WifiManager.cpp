#include "../WiFiManager.h"

#include "../../utils/Logger.h"

#include <WiFi.h>
#include <time.h>

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
    WiFi.mode(WIFI_STA);

    WiFi.disconnect(true);

    delay(500);

    Logger::Info(
        "Connecting to WiFi...");

    for (size_t i = 0; i < m_wifiCount; i++)
    {
        Logger::Info(
            "Trying SSID "
            + std::to_string(i + 1)
            + "/"
            + std::to_string(m_wifiCount)
            + " : "
            + m_wifi[i].ssid);

        WiFi.begin(
            m_wifi[i].ssid.c_str(),
            m_wifi[i].password.c_str());

        uint32_t start = millis();

        while (millis() - start < 10000)
        {
            if (WiFi.status() == WL_CONNECTED)
            {
                Logger::Info(
                    "WiFi Connected.");

                Logger::Info(
                    "SSID : " +
                    std::string(WiFi.SSID().c_str()));

                Logger::Info(
                    "IP   : " +
                    std::string(WiFi.localIP().toString().c_str()));

                Logger::Info(
                    "RSSI : " +
                    std::to_string(WiFi.RSSI()));

                //
                // Synchronize system clock
                //
                Logger::Info(
                    "Synchronizing time...");

                configTime(
                    19800,             // UTC+5:30
                    0,
                    "pool.ntp.org",
                    "time.nist.gov");

                time_t now = 0;

                uint32_t syncStart = millis();

                while (millis() - syncStart < 15000)
                {
                    time(&now);

                    if (now > 1700000000)
                        break;

                    delay(500);
                }

                if (now > 1700000000)
                {
                    Logger::Info(
                        "Time synchronized.");

                    Logger::Info(
                        "Unix Time : " +
                        std::to_string(now));

                    struct tm timeinfo;

                    if (localtime_r(&now, &timeinfo))
                    {
                        char buffer[32];

                        strftime(
                            buffer,
                            sizeof(buffer),
                            "%d-%m-%Y %H:%M:%S",
                            &timeinfo);

                        Logger::Info(
                            "Local Time : "
                            + std::string(buffer));
                    }
                }
                else
                {
                    Logger::Warning(
                        "Time synchronization timed out.");
                }

                return true;
            }

            delay(500);
        }

        Logger::Warning(
            "Failed to connect to "
            + m_wifi[i].ssid);

        WiFi.disconnect(true);

        delay(500);
    }

    Logger::Error(
        "Unable to connect to any configured WiFi.");

    return false;
}

bool WiFiManager::IsConnected() const
{
    return WiFi.status() == WL_CONNECTED;
}

void WiFiManager::MaintainConnection()
{
    if (WiFi.status() == WL_CONNECTED)
        return;

    Logger::Warning(
        "WiFi disconnected.");

    Connect();
}