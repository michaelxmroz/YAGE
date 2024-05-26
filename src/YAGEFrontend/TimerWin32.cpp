#include "TimerWin32.h"
#include "synchapi.h"

namespace
{
    inline void WinSleep(uint32_t ms) {
        if (ms == 0) {
            YieldProcessor();
        }
        else {
            Sleep(ms);
        }
    }
}

TimerWin32::TimerWin32() : m_elapsedMicroseconds(), m_endingTime(), m_frequency(), m_previous(), m_startTime()
{
}

TimerWin32::~TimerWin32()
{
	timeEndPeriod(1);
}

void TimerWin32::Start()
{
    QueryPerformanceFrequency(&m_frequency);
	timeBeginPeriod(1);
	QueryPerformanceCounter(&m_startTime);
    m_previous = Query();
}

int64_t TimerWin32::Query()
{
	QueryPerformanceCounter(&m_endingTime);

	m_endingTime.QuadPart *= 1000000ll;
	m_endingTime.QuadPart /= m_frequency.QuadPart;
	return m_endingTime.QuadPart;
}
/*
int64_t TimerWin32::GetElapsedMs()
{
	return static_cast<float>(static_cast<double>(m_elapsedMicroseconds.QuadPart) / 1000.0);
}
*/
//Based on https://nkga.github.io/post/frame-pacing-analysis-of-the-game-loop/
void TimerWin32::Limit(int64_t microSeconds)
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
                WinSleep(ms);
            }
            else 
            {
                WinSleep(0);
            }
        }
    }

    m_previous = now;
}
