#pragma once

#include <vector>
#include <cstdint>

enum class FloatFormat
{
    Standard,          // ABCD
    ByteSwapped,       // BADC
    WordSwapped,       // CDAB
    ReverseByteOrder   // DCBA
};

class ByteBuffer
{
public:

    ByteBuffer();

    explicit ByteBuffer(
        const std::vector<uint8_t>& data);

    void SetData(
        const std::vector<uint8_t>& data);

    const std::vector<uint8_t>& Data() const;

    size_t Size() const;

    uint8_t operator[](size_t index) const;

    float ReadFloat(
        FloatFormat format,
        size_t offset = 0) const;

    uint16_t ReadUInt16(
        size_t offset = 0) const;

    uint32_t ReadUInt32(
        size_t offset = 0) const;

private:

    std::vector<uint8_t> m_data;
};