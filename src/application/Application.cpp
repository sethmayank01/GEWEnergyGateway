#include "Application.h"

#include "gateway/Gateway.h"
#include "utils/Logger.h"

Application::Application()
{

}

int Application::Run()
{
    
   

    Gateway gateway;

    gateway.Initialize();

    gateway.Run();

    Logger::Info("Gateway Shutdown");

    return 0;
}