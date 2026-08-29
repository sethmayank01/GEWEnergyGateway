#pragma once

#include <string>

struct ServerCommand
{
    uint32_t id = 0;

    std::string command;

    // Raw JSON object supplied by the server. Parameterless commands receive
    // "{}", preserving compatibility with SEND_LOGS.
    std::string parameters = "{}";
};
