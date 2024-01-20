#pragma once

#define WIN32_BACKEND 1

#if WIN32_BACKEND
#include "TimerWin32.h"
typedef TimerWin32 TimerBackend;
#endif

class Clock
{
public:
	void Start();
	void Stop();
	float GetElapsedMs();
	void Sleep(float ms);
private:
	TimerBackend m_backend;
};

