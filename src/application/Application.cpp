#include "Application.h"

#include "gateway/Gateway.h"
#include "provisioning/ProvisioningManager.h"
#include "utils/Logger.h"


Application::Application()
{

}

int Application::Run()
{
#ifdef PLATFORM_WINDOWS
    Logger::Initialize();
#endif

    ProvisioningManager provisioning;
    if (!provisioning.EnsureProvisioned())
    {
        Logger::Error("Gateway provisioning failed.");
        return 1;
    }

    Gateway gateway;

    if (!gateway.Initialize())
    {
        Logger::Error("Gateway initialization failed.");
        return 1;
    }

    gateway.Run();

    Logger::Info("Gateway Shutdown");

    return 0;
}
