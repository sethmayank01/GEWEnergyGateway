#pragma once

#include <cstdint>
#include <vector>
#include <string>

class ISerialPort
{
public:

    virtual ~ISerialPort() = default;

    virtual bool Open() = 0;

    virtual void Close() = 0;

    virtual void Flush() = 0;

    virtual bool IsOpen() const = 0;

    virtual bool Write(const std::vector<uint8_t>& data) = 0;

    virtual bool Read(std::vector<uint8_t>& data,
                      size_t bytesToRead,
                      uint32_t timeoutMs) = 0;
};