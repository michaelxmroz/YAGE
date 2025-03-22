#pragma once

#include "PlatformDefines.h"

#if YAGE_PLATFORM_UNIX

#include <cstdint>
#include <time.h>

class TimerLinux
{
public:
    TimerLinux();
    ~TimerLinux();

    void Start();
    int64_t Query();
    void Limit(int64_t microSeconds);

private:
    struct timespec m_startTime;
    struct timespec m_endingTime;
    int64_t m_previous;
};

#endif // YAGE_PLATFORM_UNIX 