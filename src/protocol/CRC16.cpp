#include "CRC16.h"

uint16_t CRC16::Calculate(const std::vector<uint8_t>& data)
{
    uint16_t crc = 0xFFFF;

    for (uint8_t byte : data)
    {
        crc ^= byte;

        for (int i = 0; i < 8; i++)
        {
            if (crc & 0x0001)
            {
                crc >>= 1;
                crc ^= 0xA001;
            }
            else
            {
                crc >>= 1;
            }
        }
    }

    return crc;
}

void CRC16::Append(std::vector<uint8_t>& frame)
{
    uint16_t crc = Calculate(frame);

    frame.push_back(static_cast<uint8_t>(crc & 0xFF));

    frame.push_back(static_cast<uint8_t>((crc >> 8) & 0xFF));
}

bool CRC16::Verify(const std::vector<uint8_t>& frame)
{
    if (frame.size() < 3)
        return false;

    std::vector<uint8_t> data(frame.begin(), frame.end() - 2);

    uint16_t calculated = Calculate(data);

    uint16_t received =
        static_cast<uint16_t>(frame[frame.size() - 2]) |
        (static_cast<uint16_t>(frame[frame.size() - 1]) << 8);

    return calculated == received;
}