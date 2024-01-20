#pragma once

#include <windows.h>
#include <profileapi.h>

class TimerWin32
{
public:
	void Start();
	void Stop();
	float GetElapsedMs();
	void Sleep(float ms);

private:
	LARGE_INTEGER m_startTime;
	LARGE_INTEGER m_endingTime;
	LARGE_INTEGER m_elapsedMicroseconds;
	LARGE_INTEGER m_frequency;
};