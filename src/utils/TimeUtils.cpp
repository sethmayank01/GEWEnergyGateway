#include "TimeUtils.h"

#include <chrono>


uint64_t TimeUtils::UnixTimestamp()
{
    return static_cast<uint64_t>(
        std::chrono::duration_cast<
            std::chrono::seconds
        >(
            std::chrono::system_clock::now()
            .time_since_epoch()
        ).count()
    );
}