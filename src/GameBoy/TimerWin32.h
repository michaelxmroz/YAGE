#pragma once

#include <windows.h>
#include <profileapi.h>
#include <cstdint>

class TimerWin32
{
public:
	TimerWin32();
	~TimerWin32();

	void Start();
	int64_t Query();
	//int64_t GetElapsedMs();
	void Limit(int64_t microSeconds);

private:
	LARGE_INTEGER m_startTime;
	LARGE_INTEGER m_endingTime;
	LARGE_INTEGER m_elapsedMicroseconds;
	LARGE_INTEGER m_frequency;
	int64_t m_previous;
};