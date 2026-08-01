#pragma once

#include <string>

enum class ModbusException
{
    None,

    Timeout,

    CRCError,

    InvalidSlave,

    InvalidFunction,

    ExceptionResponse,

    CommunicationError
};

inline std::string ToString(ModbusException ex)
{
    switch (ex)
    {
        case ModbusException::None:
            return "Success";

        case ModbusException::Timeout:
            return "Timeout";

        case ModbusException::CRCError:
            return "CRC Error";

        case ModbusException::InvalidSlave:
            return "Invalid Slave ID";

        case ModbusException::InvalidFunction:
            return "Invalid Function Code";

        case ModbusException::ExceptionResponse:
            return "Modbus Exception Response";

        case ModbusException::CommunicationError:
            return "Communication Error";

        default:
            return "Unknown Error";
    }
}