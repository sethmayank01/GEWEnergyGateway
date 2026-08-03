#include "JsonBuilder.h"

#include <sstream>
#include <iomanip>

std::string JsonBuilder::Build(
    const MeterReading& reading)
{
    std::ostringstream json;

    json << std::fixed
         << std::setprecision(3);


    json
    << "{"

    << "\"gatewayId\":\""
        << reading.gatewayId
    << "\","


    << "\"device\":\""
        << reading.device
    << "\","


    << "\"firmware\":\""
        << reading.firmware
    << "\","


    << "\"timestamp\":"
        << reading.timestamp
    << ","


    << "\"measurements\":{"

        << "\"voltage\":{"
            << "\"l1\":" << reading.voltageL1 << ","
            << "\"l2\":" << reading.voltageL2 << ","
            << "\"l3\":" << reading.voltageL3
        << "},"


        << "\"current\":{"
            << "\"l1\":" << reading.currentL1 << ","
            << "\"l2\":" << reading.currentL2 << ","
            << "\"l3\":" << reading.currentL3
        << "},"


        << "\"power\":{"
            << "\"active\":" << reading.activePower << ","
            << "\"reactive\":" << reading.reactivePower << ","
            << "\"apparent\":" << reading.apparentPower
        << "},"


        << "\"powerFactor\":{"
            << "\"average\":" << reading.powerFactorAverage << ","
            << "\"l1\":" << reading.powerFactorL1 << ","
            << "\"l2\":" << reading.powerFactorL2 << ","
            << "\"l3\":" << reading.powerFactorL3
        << "},"


        << "\"frequency\":"
        << reading.frequency
        << ","


        << "\"energyReceivedWh\":"
        << reading.energyReceivedWh


    << "}"

    << "}";


    return json.str();
}