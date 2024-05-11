#pragma once

#define WIN32_BACKEND 1

#if WIN32_BACKEND
#include "TimerWin32.h"
typedef TimerWin32 TimerBackend;
#endif

class Clock
{
public:
	Clock();
	void Start();
	int64_t Query();
	//float GetElapsedMs();
	void Limit(int64_t microSeconds);
private:
	TimerBackend m_backend;
};

