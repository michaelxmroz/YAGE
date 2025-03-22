#include "TimerLinux.h"

#if YAGE_PLATFORM_UNIX

#include <unistd.h>
#include <time.h>
#include <pthread.h>

namespace
{
    void LinuxSleep(uint32_t ms) 
    {
        if (ms == 0) 
        {
            // Yield the processor - equivalent to Windows YieldProcessor
            pthread_yield();
        }
        else 
        {
            struct timespec ts;
            ts.tv_sec = ms / 1000;
            ts.tv_nsec = (ms % 1000) * 1000000;
            nanosleep(&ts, NULL);
        }
    }

    int64_t timespec_to_microseconds(const struct timespec& ts) 
    {
        return (ts.tv_sec * 1000000LL) + (ts.tv_nsec / 1000LL);
    }
}

TimerLinux::TimerLinux() : m_startTime{0}, m_endingTime{0}, m_previous(0)
{
}

TimerLinux::~TimerLinux()
{
    // Nothing to clean up in Linux implementation
}

void TimerLinux::Start()
{
    clock_gettime(CLOCK_MONOTONIC, &m_startTime);
    m_previous = Query();
}

int64_t TimerLinux::Query()
{
    clock_gettime(CLOCK_MONOTONIC, &m_endingTime);
    return timespec_to_microseconds(m_endingTime);
}

void TimerLinux::Limit(int64_t microSeconds)
{
    int64_t now = 0;

    if (microSeconds > 0) 
    {
        for (;;) 
        {
            now = Query();

            int64_t deltaUs = (now - m_previous);
            if (deltaUs >= microSeconds) 
            {
                break;
            }

            int64_t sleepUs = (microSeconds - deltaUs);
            if (sleepUs > 2000) 
            {
                uint32_t ms = static_cast<uint32_t>((sleepUs - 2000) / 1000);
                LinuxSleep(ms);
            }
            else 
            {
                LinuxSleep(0);
            }
        }
    }

    m_previous = now;
}

#endif // YAGE_PLATFORM_UNIX 