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

namespace Helpers
{
	FORCE_INLINE bool IsTimerEnabled(Memory& memory)
	{
		return (memory[TAC_REGISTER] & 0x4) > 0;
	}

	FORCE_INLINE uint16_t GetTimerFrequency(Memory& memory)
	{
		return TIMER_SPEED_BITS[memory[TAC_REGISTER] & 0x3];
	}
}


Timer::Timer()
	: Timer(nullptr)
{
}

Timer::Timer(Serializer* serializer)
	: ISerializable(serializer)
	, m_previousCycleTimerModuloEdge(false)
	, m_divTotal(0)
{
}

void Timer::Init(Memory& memory)
{
	m_divHigh = 0x18;
	m_divLow = 0xCC;

	memory.Write(DIVIDER_REGISTER, 0x18);
	memory.Write(TIMA_REGISTER, 0x00);
	memory.Write(TMA_REGISTER, 0x00);
	memory.Write(TAC_REGISTER, 0xF8);
	memory.RegisterCallback(DIVIDER_REGISTER, Timer::ResetDivider, this);

	memory.AddIOUnusedBitsOverride(TAC_REGISTER, 0b11111000);
}

void Timer::Increment(uint32_t mCycles, Memory& memory)
{
	uint32_t cyclesToStep = mCycles * MCYCLES_TO_CYCLES;

	bool isTimerEnabled = Helpers::IsTimerEnabled(memory);
	uint16_t timerBits = Helpers::GetTimerFrequency(memory);

	while (cyclesToStep > 0)
	{
		m_divTotal++;
		if (m_divLow == 0) // overflow
		{
			memory.WriteDirect(DIVIDER_REGISTER, m_divHigh);
		}

		bool currentFrequencyEdge = (m_divTotal & timerBits) && isTimerEnabled;
		if (m_previousCycleTimerModuloEdge && !currentFrequencyEdge)
		{
			uint8_t TIMA = memory[TIMA_REGISTER];
			if (TIMA == 0xFF)
			{
				memory.Write(TIMA_REGISTER, memory[TMA_REGISTER]);
				Interrupts::RequestInterrupt(Interrupts::Types::Timer, memory);
			}
			else
			{
				memory.Write(TIMA_REGISTER, TIMA + 1);
			}
		}
		m_previousCycleTimerModuloEdge = currentFrequencyEdge;

		cyclesToStep--;
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

void Timer::Serialize(std::vector<Chunk>& chunks, std::vector<uint8_t>& data)
{
	uint32_t dataSize = sizeof(uint16_t) + sizeof(bool);
	uint8_t* rawData = CreateChunkAndGetDataPtr(chunks, data, dataSize, ChunkId::Timer);

	WriteAndMove(rawData, &m_divTotal, sizeof(uint16_t));
	WriteAndMove(rawData, &m_previousCycleTimerModuloEdge, sizeof(bool));
}

void Timer::Deserialize(const Chunk* chunks, const uint32_t& chunkCount, const uint8_t* data, const uint32_t& dataSize)
{
	const Chunk* myChunk = FindChunk(chunks, chunkCount, ChunkId::Timer);
	if (myChunk == nullptr)
	{
		return;
	}

	data += myChunk->m_offset;

	ReadAndMove(data, &m_divTotal, sizeof(uint16_t));
	ReadAndMove(data, &m_previousCycleTimerModuloEdge, sizeof(bool));
}
