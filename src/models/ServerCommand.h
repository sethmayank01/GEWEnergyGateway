#pragma once

#include <string>

struct ServerCommand
{
    uint32_t id = 0;

    std::string command;

    std::string value;
};