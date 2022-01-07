#pragma once
#include <cstdint>
#include "Memory.h"

class Clock
{
public:
	Clock();

	void Increment(uint32_t mCycles, Memory& memory);

	void Reset();
	static void ResetDivider(Memory& memory);
	static void CheckForDividerWrite(uint16_t addr, Memory& memory);
private:
	uint32_t m_dividerCycleAccumulator;
	uint32_t m_timerCycleAccumulator;
};

