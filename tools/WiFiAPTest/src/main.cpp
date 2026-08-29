#include <Arduino.h>
#include <WebServer.h>
#include <WiFi.h>

namespace
{
    constexpr const char* TEST_SSID = "GEW-AP-OPEN-TEST";
    constexpr uint8_t TEST_CHANNEL = 6;

    WebServer server(80);
}

void setup()
{
    Serial.begin(115200);
    delay(1500);

    Serial.println();
    Serial.println("Starting minimal ESP32-S3 AP test...");

    WiFi.mode(WIFI_OFF);
    delay(500);
    WiFi.mode(WIFI_AP);
    delay(500);

    const bool started = WiFi.softAP(TEST_SSID, nullptr, TEST_CHANNEL, false, 4);

    server.on("/", []() {
        server.send(200, "text/plain", "GEW ESP32 AP test OK\n");
    });
    server.onNotFound([]() {
        server.send(200, "text/plain", "GEW ESP32 AP test OK\n");
    });
    server.begin();

    Serial.printf("AP started: %s\n", started ? "YES" : "NO");
    Serial.printf("SSID: %s\n", TEST_SSID);
    Serial.println("Security: OPEN (diagnostic only)");
    Serial.printf("Channel: %u\n", TEST_CHANNEL);
    Serial.printf("AP MAC: %s\n", WiFi.softAPmacAddress().c_str());
    Serial.printf("AP IP: %s\n", WiFi.softAPIP().toString().c_str());
}

void loop()
{
    server.handleClient();

    static int previousStationCount = -1;
    const int stationCount = WiFi.softAPgetStationNum();
    if (stationCount != previousStationCount)
    {
        Serial.printf("Connected stations: %d\n", stationCount);
        previousStationCount = stationCount;
    }
    delay(1000);
}
