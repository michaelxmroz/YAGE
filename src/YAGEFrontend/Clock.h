#pragma once

#include "PlatformDefines.h"
#include "Timer.h"

class Clock
{
public:
	Clock();
	void Start();
	int64_t Query();
	//float GetElapsedMs();
	void Limit(int64_t microSeconds);
private:
	Timer m_backend;
};

