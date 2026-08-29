#pragma once

class ProvisioningManager
{
public:
    // Ensures network access and a valid gateway configuration. On ESP32 this
    // may run the captive setup portal and contact the provisioning server.
    bool EnsureProvisioned();
};
