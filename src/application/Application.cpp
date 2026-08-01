#include "Application.h"

#include "gateway/Gateway.h"
#include "utils/Logger.h"

Application::Application()
{

}

int Application::Run()
{
    Logger::Info("------------------------------------");
    Logger::Info("GEW Energy Gateway");
    Logger::Info("Version 0.1.0");
    Logger::Info("------------------------------------");

    Gateway gateway;

    gateway.Initialize();

    gateway.Run();

    Logger::Info("Gateway Shutdown");

    return 0;
}