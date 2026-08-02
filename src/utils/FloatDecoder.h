#include <vector>

enum class FloatFormat
{
    ABCD,
    BADC,
    CDAB,
    DCBA
};

class FloatDecoder
{
public:
    static float Decode(
        const std::vector<uint8_t>& data,
        FloatFormat format);
};