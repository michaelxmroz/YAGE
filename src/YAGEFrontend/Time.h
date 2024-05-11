#pragma once
#define WIN32_LEAN_AND_MEAN
#include <cstdint>

#include "windows.h"

#define DAMPEN_ACCUMULATOR 10

class Time
{
public:
	Time();
	void StartFrame();
	void ResetLastStepTime(double preferredStepTime);
	void ResetLastStepTime();
	double GetDeltaTime();
	double GetDampenedDeltaTime();
	double GetTimeSinceStartup();
	double GetTimeSinceLastStep();

private:
	double m_deltaAccumulator[DAMPEN_ACCUMULATOR];
	double m_timeSinceStartup;
	double m_deltaTime;
	double m_dampenedDeltaTime;
	double m_timeSinceLastStep;
	LARGE_INTEGER m_previousTime;
	LARGE_INTEGER m_currentTime;
	LARGE_INTEGER m_startTime;
	LARGE_INTEGER m_elapsedMicroseconds;
	LARGE_INTEGER m_frequency;
	uint32_t m_accumulatorPosition;
};