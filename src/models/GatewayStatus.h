#pragma once

enum class GatewayStatus
{
    Stopped,
    Starting,
    Running,
    CloudDisconnected,
    CloudConnected,
    Error
};