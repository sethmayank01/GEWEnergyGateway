#include "ByteBuffer.h"

#include <cstring>

ByteBuffer::ByteBuffer()
{
}

ByteBuffer::ByteBuffer(
    const std::vector<uint8_t>& data)
{
    m_data = data;
}

void ByteBuffer::SetData(
    const std::vector<uint8_t>& data)
{
    m_data = data;
}

const std::vector<uint8_t>&
ByteBuffer::Data() const
{
    return m_data;
}

size_t ByteBuffer::Size() const
{
    return m_data.size();
}

uint8_t
ByteBuffer::operator[](size_t index) const
{
    return m_data[index];
}

float ByteBuffer::ReadFloat(
    FloatFormat format,
    size_t offset) const
{
    if (offset + 4 > m_data.size())
        return 0.0f;

    uint8_t bytes[4];

    switch (format)
    {
    case FloatFormat::Standard:

        bytes[0] = m_data[offset + 0];
        bytes[1] = m_data[offset + 1];
        bytes[2] = m_data[offset + 2];
        bytes[3] = m_data[offset + 3];
        break;

    case FloatFormat::ByteSwapped:

        bytes[0] = m_data[offset + 1];
        bytes[1] = m_data[offset + 0];
        bytes[2] = m_data[offset + 3];
        bytes[3] = m_data[offset + 2];
        break;

    case FloatFormat::WordSwapped:

        bytes[0] = m_data[offset + 2];
        bytes[1] = m_data[offset + 3];
        bytes[2] = m_data[offset + 0];
        bytes[3] = m_data[offset + 1];
        break;

    case FloatFormat::ReverseByteOrder:

        bytes[0] = m_data[offset + 3];
        bytes[1] = m_data[offset + 2];
        bytes[2] = m_data[offset + 1];
        bytes[3] = m_data[offset + 0];
        break;
    }

    float value;

    std::memcpy(&value, bytes, sizeof(float));

    return value;
}

uint16_t ByteBuffer::ReadUInt16(
    size_t offset) const
{
    if (offset + 2 > m_data.size())
        return 0;

    return
        (static_cast<uint16_t>(m_data[offset]) << 8) |
        (static_cast<uint16_t>(m_data[offset + 1]));
}

uint32_t ByteBuffer::ReadUInt32(
    size_t offset) const
{
    if (offset + 4 > m_data.size())
        return 0;

    return
        (static_cast<uint32_t>(m_data[offset]) << 24) |
        (static_cast<uint32_t>(m_data[offset + 1]) << 16) |
        (static_cast<uint32_t>(m_data[offset + 2]) << 8) |
        (static_cast<uint32_t>(m_data[offset + 3]));
}