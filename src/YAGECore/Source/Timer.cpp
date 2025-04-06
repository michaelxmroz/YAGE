#include "Timer.h"
#include "Helpers.h"
#include "Interrupts.h"

#define DIVIDER_REGISTER 0xFF04
#define TIMA_REGISTER 0xFF05
#define TMA_REGISTER 0xFF06
#define TAC_REGISTER 0xFF07

const uint32_t TIMER_SPEED_BITS[4]
{
	512,
	8,
	32,
	128
};

namespace
{
	bool IsTimerEnabled(Memory& memory)
	{
		return (memory[TAC_REGISTER] & 0x4) > 0;
	}

	uint16_t GetTimerFrequency(Memory& memory)
	{
		return TIMER_SPEED_BITS[memory[TAC_REGISTER] & 0x3];
	}
}


Timer::Timer()
	: Timer(nullptr)
{
}

Timer::Timer(GamestateSerializer* serializer)
	: ISerializable(serializer, ChunkId::Timer)
	, m_previousCycleTimerModuloEdge(false)
	, m_divTotal(0)
	, m_TIMAReloadState(TIMAReloadState::None)
{
}

void Timer::Init(Memory& memory)
{
	m_divHigh = 0xAB;
	m_divLow = 0xC8;

	memory.Write(DIVIDER_REGISTER, m_divHigh);
	memory.Write(TIMA_REGISTER, 0x00);
	memory.Write(TMA_REGISTER, 0x00);
	memory.Write(TAC_REGISTER, 0xF8);
	memory.RegisterCallback(DIVIDER_REGISTER, Timer::ResetDivider, this);
	memory.RegisterCallback(TIMA_REGISTER, Timer::WriteTIMA, this);
	memory.RegisterCallback(TMA_REGISTER, Timer::WriteTMA, this);
	memory.RegisterCallback(TAC_REGISTER, Timer::WriteTAC, this);

	memory.AddIOUnusedBitsOverride(TAC_REGISTER, 0b11111000);
}

void Timer::Increment(uint32_t mCycles, Memory& memory)
{
	uint32_t cyclesToStep = mCycles * MCYCLES_TO_CYCLES;

	bool isTimerEnabled = IsTimerEnabled(memory);
	uint16_t timerBits = GetTimerFrequency(memory);

	switch (m_TIMAReloadState)
	{
	case TIMAReloadState::Overflowed:
		memory.WriteDirect(TIMA_REGISTER, memory[TMA_REGISTER]);
		Interrupts::RequestInterrupt(Interrupts::Types::Timer, memory);
		m_TIMAReloadState = TIMAReloadState::InterruptTriggered;
		break;
	case TIMAReloadState::InterruptTriggered:
		m_TIMAReloadState = TIMAReloadState::None;
		break;
	default:
		break;
	}

	while (cyclesToStep > 0)
	{
		m_divTotal++;
		if (m_divLow == 0) // overflow
		{
			memory.WriteDirect(DIVIDER_REGISTER, m_divHigh);
		}

		CheckForTimerTick(timerBits, isTimerEnabled, memory);

		cyclesToStep--;
	}
}

void Timer::CheckForTimerTick(uint16_t timerBits, bool isTimerEnabled, Memory& memory)
{
	bool currentFrequencyEdge = (m_divTotal & timerBits) && isTimerEnabled;
	if (m_previousCycleTimerModuloEdge && !currentFrequencyEdge)
	{
		TickTimer(memory);
	}
	m_previousCycleTimerModuloEdge = currentFrequencyEdge;
}

void Timer::TickTimer(Memory& memory)
{
	uint8_t TIMA = memory[TIMA_REGISTER];
	if (TIMA == 0xFF)
	{
		memory.WriteDirect(TIMA_REGISTER, 0x00);
		m_TIMAReloadState = TIMAReloadState::Overflowed;
	}
	else
	{
		memory.WriteDirect(TIMA_REGISTER, TIMA + 1);
	}
}

void Timer::Reset()
{
	m_divTotal = 0;
	m_previousCycleTimerModuloEdge = false;
}

void Timer::ResetDivider(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue, void* userData)
{
	memory->WriteDirect(DIVIDER_REGISTER, 0);
	Timer* timer = static_cast<Timer*>(userData);
	timer->m_divTotal = 0;
}

void Timer::WriteTIMA(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue, void* userData)
{
	Timer* timer = static_cast<Timer*>(userData);
	//Hardware quirk: writing to TIMA the cycle it overflowed prevents reload from TMA + interrupt.
	if (timer->m_TIMAReloadState == TIMAReloadState::Overflowed)
	{
		timer->m_TIMAReloadState = TIMAReloadState::None;
	}
	//Hardware quirk: writing to TIMA the cycle after it overflowed is ignored.
	else if (timer->m_TIMAReloadState == TIMAReloadState::InterruptTriggered)
	{
		memory->WriteDirect(TIMA_REGISTER, prevValue);
	}
}

void Timer::WriteTMA(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue, void* userData)
{
	Timer* timer = static_cast<Timer*>(userData);

	//Hardware quirk: writing to TMA the cycle after TIMA overflowed applies the new TMA value to TIMA as well.
	if (timer->m_TIMAReloadState == TIMAReloadState::InterruptTriggered)
	{
		memory->WriteDirect(TIMA_REGISTER, newValue);
	}
}

void Timer::WriteTAC(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue, void* userData)
{
	Timer* timer = static_cast<Timer*>(userData);
	// Hardware quirk: Disabling the timer while the cycle bits were true will cause an additional tick
	if (timer->m_previousCycleTimerModuloEdge && !IsTimerEnabled(*memory))
	{
		timer->TickTimer(*memory);
		timer->m_previousCycleTimerModuloEdge = false;
	}	
}

void Timer::Serialize(uint8_t* data)
{
	WriteAndMove(data, &m_divTotal, sizeof(uint16_t));
	WriteAndMove(data, &m_previousCycleTimerModuloEdge, sizeof(bool));
	WriteAndMove(data, &m_TIMAReloadState, sizeof(uint8_t));
}

void Timer::Deserialize(const uint8_t* data)
{
	ReadAndMove(data, &m_divTotal, sizeof(uint16_t));
	ReadAndMove(data, &m_previousCycleTimerModuloEdge, sizeof(bool));
	ReadAndMove(data, &m_TIMAReloadState, sizeof(uint8_t));
}

uint32_t Timer::GetSerializationSize()
{
	return sizeof(uint16_t) + sizeof(bool);
}
