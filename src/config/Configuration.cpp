#include "Configuration.h"


#ifdef PLATFORM_WINDOWS
#include <fstream>
#include "../../thirdparty/nlohmann/json.hpp"
using json = nlohmann::json;
#else
#include <LittleFS.h>
#include <ArduinoJson.h>
#endif

#ifdef PLATFORM_WINDOWS

#define JSON_GET_STRING(obj, key, def) obj.value(key, def)
#define JSON_GET_INT(obj, key, def)    obj.value(key, def)

#else

#define JSON_GET_STRING(obj, key, def) std::string(obj[key] | def)
#define JSON_GET_INT(obj, key, def)    (obj[key] | def)

#endif

bool Configuration::Load(const std::string& filename)
{
#ifdef PLATFORM_WINDOWS

    std::ifstream file(filename);

    if (!file.is_open())
        return false;

    json j;
    file >> j;

#else
    Logger::Info("Mounting LittleFS...");
    if (!LittleFS.begin())
{
    return false;
}
    Logger::Info("LittleFS mounted.");

     if (!Logger::Initialize())
    {
        Serial.println("Logger initialization failed.");
    }

    File file = LittleFS.open(filename.c_str(), "r");

    if (!file)
        return false;

    Logger::Info("gateway.json opened successfully.");
    JsonDocument j;

    DeserializationError err = deserializeJson(j, file);

    file.close();

    if (err)
        return false;

#endif
    #ifdef PLATFORM_WINDOWS
auto gateway = j["gateway"];
#else
JsonObject gateway = j["gateway"].as<JsonObject>();
#endif

    m_config.gateway.gatewayId =
    JSON_GET_STRING(gateway, "gatewayId","");

    m_config.gateway.apiKey =
    JSON_GET_STRING(gateway, "apiKey","");

    m_config.gateway.firmware =
    JSON_GET_STRING(gateway, "firmware","");
   
    m_config.gateway.hardware =
    JSON_GET_STRING(gateway, "hardware","");
  
  #ifdef PLATFORM_WINDOWS
auto meter = j["meter"];
#else
JsonObject meter = j["meter"].as<JsonObject>();
#endif
    

     m_config.meter.manufacturer =
    JSON_GET_STRING(meter, "manufacturer","");
  
     m_config.meter.model =
    JSON_GET_STRING(meter, "model","");
  
    m_config.meter.port =
    JSON_GET_STRING(meter, "port","COM1");

    m_config.meter.baud =
    JSON_GET_INT(meter, "baud", 9600);

   std::string parity =
    JSON_GET_STRING(meter, "parity", "E");

if (!parity.empty())
    m_config.meter.parity = parity.front();
else
    m_config.meter.parity = 'E';
    
    m_config.meter.stopBits =
    JSON_GET_INT(meter, "stopBits", 1);

    m_config.meter.slaveId =
    JSON_GET_INT(meter, "slaveId", 1);

    #ifdef PLATFORM_WINDOWS
auto cloud = j["cloud"];
#else
JsonObject cloud = j["cloud"].as<JsonObject>();
#endif


    m_config.cloud.url =
    JSON_GET_STRING(cloud, "url","");

    m_config.cloud.uploadInterval =
    JSON_GET_INT(cloud, "uploadInterval", 2);
    
   #ifndef PLATFORM_WINDOWS
m_config.wifiCount = 0;
  JsonArray wifi = j["wifi"].as<JsonArray>();

    if (!wifi.isNull())
    {
        

        for (JsonObject network : wifi)
        {
            if (m_config.wifiCount >= MAX_WIFI_NETWORKS)
                break;

            auto& cred = m_config.wifi[m_config.wifiCount];

            cred.ssid =
                JSON_GET_STRING(network, "ssid", "");

            cred.password =
                JSON_GET_STRING(network, "password", "");

            if (!cred.ssid.empty())
                m_config.wifiCount++;
        }
    }
#endif
      
    return Validate();
}

bool Configuration::Validate() const
{
    if (m_config.gateway.gatewayId.empty())
        return false;

#ifdef PLATFORM_WINDOWS

    if (m_config.meter.port.empty())
        return false;

#else

    if (m_config.wifiCount == 0)
        return false;

#endif

    if (m_config.meter.baud != 1200 &&
        m_config.meter.baud != 2400 &&
        m_config.meter.baud != 4800 &&
        m_config.meter.baud != 9600 &&
        m_config.meter.baud != 19200 &&
        m_config.meter.baud != 38400 &&
        m_config.meter.baud != 57600 &&
        m_config.meter.baud != 115200)
        return false;

    if (m_config.meter.parity != 'N' &&
        m_config.meter.parity != 'E' &&
        m_config.meter.parity != 'O')
        return false;

    if (m_config.meter.stopBits < 1 ||
        m_config.meter.stopBits > 2)
        return false;

    if (m_config.cloud.uploadInterval <= 0)
        return false;

    if (m_config.cloud.url.empty())
        return false;

    return true;
}

const GatewayConfig&
Configuration::Get() const
{
    return m_config;
}