#pragma once
#include <cstdint>
#include "Memory.h"

class Clock
{
public:
	Clock();

	void Init(Memory& memory);

	void Increment(uint32_t mCycles, Memory& memory);

	void Reset();
	static void ResetDivider(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue);
private:
	uint32_t m_dividerCycleAccumulator;
	uint32_t m_timerCycleAccumulator;
};

