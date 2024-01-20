#include "Clock.h"

void Clock::Start()
{
	m_backend.Start();
}

void Clock::Stop()
{
	m_backend.Stop();
}

float Clock::GetElapsedMs()
{
	return m_backend.GetElapsedMs();
}

void Clock::Sleep(float ms)
{
	m_backend.Sleep(ms);
}
