#include "Gateway.h"

#include "utils/Logger.h"

Gateway::Gateway()
{

}

bool Gateway::Initialize()
{
    Logger::Info("Initializing Gateway...");

    return true;
}

void Gateway::Run()
{
    Logger::Info("Gateway Running...");
}