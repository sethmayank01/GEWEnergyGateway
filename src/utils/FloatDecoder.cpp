#include "FloatDecoder.h"

#include <cstring>
#include <vector>

float FloatDecoder::Decode(
    const std::vector<uint8_t>& data,
    FloatFormat format)
{
    if (data.size() != 4)
        return 0.0f;

    uint8_t bytes[4];

    switch (format)
    {
    case FloatFormat::ABCD:
        bytes[0]=data[0];
        bytes[1]=data[1];
        bytes[2]=data[2];
        bytes[3]=data[3];
        break;

    case FloatFormat::BADC:
        bytes[0]=data[1];
        bytes[1]=data[0];
        bytes[2]=data[3];
        bytes[3]=data[2];
        break;

    case FloatFormat::CDAB:
        bytes[0]=data[2];
        bytes[1]=data[3];
        bytes[2]=data[0];
        bytes[3]=data[1];
        break;

    case FloatFormat::DCBA:
        bytes[0]=data[3];
        bytes[1]=data[2];
        bytes[2]=data[1];
        bytes[3]=data[0];
        break;
    }

    float value;
    std::memcpy(&value, bytes, sizeof(float));

    return value;
}