#include "Configuration.h"

#include <fstream>

#include "../../thirdparty/nlohmann/json.hpp"

using json = nlohmann::json;

bool Configuration::Load(const std::string& filename)
{
    std::ifstream file(filename);

    if(!file.is_open())
        return false;

    json j;

    file >> j;

    auto gateway = j["gateway"];

    m_config.gateway.gatewayId =
        gateway.value("gatewayId","");

    m_config.gateway.firmware =
        gateway.value("firmware","");

    m_config.gateway.hardware =
        gateway.value("hardware","");

    auto meter = j["meter"];

    m_config.meter.manufacturer =
        meter.value("manufacturer","");

    m_config.meter.model =
        meter.value("model","");

    m_config.meter.port =
        meter.value("port","COM1");

    m_config.meter.baud =
        meter.value("baud",9600);

    std::string parity =
        meter.value("parity","E");

    m_config.meter.parity =
        parity.empty() ? 'E' : parity[0];

    m_config.meter.stopBits =
        meter.value("stopBits",1);

    m_config.meter.slaveId =
        meter.value("slaveId",1);

    auto cloud = j["cloud"];

    m_config.cloud.url =
        cloud.value("url","");

    m_config.cloud.uploadInterval =
        cloud.value("uploadInterval",2);

    m_config.cloud.apiKey =
        cloud.value("apiKey","");

    return Validate();
}

bool Configuration::Validate() const
{
    if(m_config.gateway.gatewayId.empty())
        return false;

    if(m_config.meter.port.empty())
        return false;

    if(m_config.meter.baud != 1200 &&
       m_config.meter.baud != 2400 &&
       m_config.meter.baud != 4800 &&
       m_config.meter.baud != 9600 &&
       m_config.meter.baud != 19200 &&
       m_config.meter.baud != 38400 &&
       m_config.meter.baud != 57600 &&
       m_config.meter.baud != 115200)
        return false;

    if(m_config.meter.parity!='N' &&
       m_config.meter.parity!='E' &&
       m_config.meter.parity!='O')
        return false;

    if(m_config.meter.stopBits<1 ||
       m_config.meter.stopBits>2)
        return false;

    if(m_config.cloud.uploadInterval<=0)
        return false;
    
    if(m_config.cloud.url.empty())
        return false;

    return true;
}

const GatewayConfig&
Configuration::Get() const
{
    return m_config;
}