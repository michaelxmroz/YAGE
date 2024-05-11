#include "Time.h"
#include <cstdint>

Time::Time()
{
	QueryPerformanceFrequency(&m_frequency);
	QueryPerformanceCounter(&m_currentTime);
	m_startTime = m_currentTime;
	m_timeSinceLastStep = 0.0;
	m_dampenedDeltaTime = m_deltaTime;
	m_accumulatorPosition = 0;
	for(uint32_t i = 0; i < DAMPEN_ACCUMULATOR; ++i)
	{
		m_deltaAccumulator[i] = 0.0;
	}
}

void Time::StartFrame()
{
	m_previousTime = m_currentTime;
	QueryPerformanceCounter(&m_currentTime);

	m_elapsedMicroseconds.QuadPart = m_currentTime.QuadPart - m_previousTime.QuadPart;
	m_elapsedMicroseconds.QuadPart *= 1000000;
	m_elapsedMicroseconds.QuadPart /= m_frequency.QuadPart;

	m_deltaTime = static_cast<double>(m_elapsedMicroseconds.QuadPart) / 1000.0;

	m_elapsedMicroseconds.QuadPart = m_currentTime.QuadPart - m_startTime.QuadPart;
	m_elapsedMicroseconds.QuadPart *= 1000000;
	m_elapsedMicroseconds.QuadPart /= m_frequency.QuadPart;

	m_timeSinceStartup = static_cast<double>(m_elapsedMicroseconds.QuadPart) / 1000.0;
	m_timeSinceLastStep += m_deltaTime;

	m_deltaAccumulator[m_accumulatorPosition] = m_deltaTime;

	m_dampenedDeltaTime = 0;
	for (uint32_t i = 0; i < DAMPEN_ACCUMULATOR; ++i)
	{
		m_dampenedDeltaTime += m_deltaAccumulator[i];
	}
	m_dampenedDeltaTime /= DAMPEN_ACCUMULATOR;

	m_accumulatorPosition++;
	if(m_accumulatorPosition >= DAMPEN_ACCUMULATOR)
	{
		m_accumulatorPosition = 0;
	}
}

void Time::ResetLastStepTime(double preferredStepTime)
{
	m_timeSinceLastStep -= preferredStepTime;
}

void Time::ResetLastStepTime()
{
	m_timeSinceLastStep = 0;
}

double Time::GetTimeSinceLastStep()
{
	return m_timeSinceLastStep;
}


double Time::GetDeltaTime()
{
	return m_deltaTime;
}

double Time::GetDampenedDeltaTime()
{
	return m_dampenedDeltaTime;
}

double Time::GetTimeSinceStartup()
{
	return m_timeSinceStartup;
}
