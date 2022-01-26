#include "Clock.h"
#include "Helpers.h"
#include "Interrupts.h"

#define CPU_FREQUENCY 4194304
#define DIVIDER_MCYCLES 64
#define FREQUENCY_TO_MCYCLES 4

#define DIVIDER_REGISTER 0xFF04
#define TIMA_REGISTER 0xFF05
#define TMA_REGISTER 0xFF06
#define TAC_REGISTER 0xFF07

const uint32_t TIMER_SPEEDS[4]
{
	256,
	4,
	16,
	64
};

namespace Helpers
{
	FORCE_INLINE bool IsTimerEnabled(Memory& memory)
	{
		return (memory[TAC_REGISTER] & 0x4) > 0;
	}

	FORCE_INLINE uint32_t GetTimerFrequency(Memory& memory)
	{
		return TIMER_SPEEDS[memory[TAC_REGISTER] & 0x3];
	}
}


Clock::Clock() 
	: m_dividerCycleAccumulator(0)
	, m_timerCycleAccumulator(0)
{
}

void Clock::Init(Memory& memory)
{
	memory.Write(DIVIDER_REGISTER, 0x00);
	memory.Write(TAC_REGISTER, 0xF8);
	memory.RegisterCallback(DIVIDER_REGISTER, Clock::ResetDivider);
}

void Clock::Increment(uint32_t mCycles, Memory& memory)
{
	m_dividerCycleAccumulator += mCycles;
	if (m_dividerCycleAccumulator >= DIVIDER_MCYCLES)
	{
		m_dividerCycleAccumulator -= DIVIDER_MCYCLES;
		memory.WriteDirect(DIVIDER_REGISTER, memory[DIVIDER_REGISTER] + 1);
	}

	if (Helpers::IsTimerEnabled(memory))
	{
		m_timerCycleAccumulator += mCycles;
		uint32_t timerFrequency = Helpers::GetTimerFrequency(memory);
		if (m_dividerCycleAccumulator >= timerFrequency)
		{
			m_dividerCycleAccumulator %= timerFrequency;
			if (memory[TIMA_REGISTER] == 0xFF)
			{
				memory.Write(TIMA_REGISTER, memory[TMA_REGISTER]);
				Interrupts::RequestInterrupt(Interrupts::Types::Timer, memory);
			}
			else
			{
				memory.Write(TIMA_REGISTER, memory[TIMA_REGISTER] + 1);
			}
		}
	}
}

void Clock::Reset()
{
	m_dividerCycleAccumulator = 0;
	m_timerCycleAccumulator = 0;
}

void Clock::ResetDivider(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue)
{
	memory->WriteDirect(DIVIDER_REGISTER, 0);
}
