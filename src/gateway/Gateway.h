#pragma once
#include "../config/Configuration.h"
#include "../network/WiFiManager.h"

class Gateway
{
public:

    Gateway();

    bool Initialize();

    void Run();
private:
    Configuration m_config;

    #ifdef PLATFORM_ESP32
    WiFiManager* m_wifi = nullptr;
#endif

};