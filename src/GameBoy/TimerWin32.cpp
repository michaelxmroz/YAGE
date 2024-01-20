#include "TimerWin32.h"
#include "synchapi.h"

void TimerWin32::Start()
{
	QueryPerformanceFrequency(&m_frequency);
	QueryPerformanceCounter(&m_startTime);
}

void TimerWin32::Stop()
{
	QueryPerformanceCounter(&m_endingTime);
	m_elapsedMicroseconds.QuadPart = m_endingTime.QuadPart - m_startTime.QuadPart;

	m_elapsedMicroseconds.QuadPart *= 1000000;
	m_elapsedMicroseconds.QuadPart /= m_frequency.QuadPart;
}

float TimerWin32::GetElapsedMs()
{
	return static_cast<float>(static_cast<double>(m_elapsedMicroseconds.QuadPart) / 1000.0);
}

void TimerWin32::Sleep(float ms)
{
	SleepEx(ms, false);
}
