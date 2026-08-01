#include "HexDump.h"

#include <iomanip>
#include <sstream>

std::string HexDump::ToString(const std::vector<uint8_t>& data)
{
    std::ostringstream ss;

    ss << std::uppercase << std::hex << std::setfill('0');

    for (size_t i = 0; i < data.size(); i++)
    {
        ss << std::setw(2) << static_cast<int>(data[i]);

        if (i + 1 < data.size())
            ss << " ";
    }

    return ss.str();
}