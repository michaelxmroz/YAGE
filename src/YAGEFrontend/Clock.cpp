#include "Clock.h"

Clock::Clock() :
	m_backend()
{

}

void Clock::Start()
{
	m_backend.Start();
}

int64_t Clock::Query()
{
	return m_backend.Query();
}
/*
float Clock::GetElapsedMs()
{
	return m_backend.GetElapsedMs();
}
*/
void Clock::Limit(int64_t microSeconds)
{
	m_backend.Limit(microSeconds);
}
