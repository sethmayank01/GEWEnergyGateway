#pragma once

#include "../models/GatewayConfig.h"

#include <string>

class Configuration
{
public:

    bool Load(const std::string& filename);

    bool Validate() const;

    const GatewayConfig& Get() const;

private:

    GatewayConfig m_config;
};