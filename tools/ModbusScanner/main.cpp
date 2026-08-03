#include "config/Configuration.h"
#include "platform/windows/SerialPortWin.h"
#include "protocol/ModbusRTU.h"
#include "utils/Logger.h"
#include <thread>
#include <chrono>

int main()
{
    Configuration cfg;

    if(!cfg.Load("../gateway.json"))
    {
        Logger::Error("Cannot load gateway.json");
        return -1;
    }

    SerialPortWin serial(cfg.Get().meter);

    if(!serial.Open())
    {
        Logger::Error("Cannot open COM Port");
        return -1;
    }

    ModbusRTU modbus(serial);

    for(uint16_t reg = 132; reg <= 160; reg += 2)
    {
        std::vector<uint8_t> payload;

        Logger::Info("--------------------------------");
        Logger::Info("Register : " + std::to_string(reg));

        auto result =
            modbus.ReadHoldingRegisters(
                cfg.Get().meter.slaveId,
                reg,
                2,
                payload);

        if(result == ModbusException::None)
        {
            Logger::Info("SUCCESS");

            Logger::Hex("DATA:", payload);
        }
        else
        {
            Logger::Error(ToString(result));
        }
        std::this_thread::sleep_for(
    std::chrono::milliseconds(500));
    }

    serial.Close();

    return 0;
}