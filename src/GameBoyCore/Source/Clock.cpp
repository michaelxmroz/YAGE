#include "Clock.h"
#include "Helpers.h"
#include "Interrupts.h"

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
	: Clock(nullptr)
{
}

Clock::Clock(Serializer* serializer)
	: ISerializable(serializer)
	, m_dividerCycleAccumulator(0)
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

void Clock::Serialize(std::vector<Chunk>& chunks, std::vector<uint8_t>& data)
{
	uint32_t dataSize = sizeof(uint32_t) + sizeof(uint32_t);
	uint8_t* rawData = CreateChunkAndGetDataPtr(chunks, data, dataSize, ChunkId::Clock);

	WriteAndMove(rawData, &m_dividerCycleAccumulator, sizeof(uint32_t));
	WriteAndMove(rawData, &m_timerCycleAccumulator, sizeof(uint32_t));
}

void Clock::Deserialize(const Chunk* chunks, const uint32_t& chunkCount, const uint8_t* data, const uint32_t& dataSize)
{
	const Chunk* myChunk = FindChunk(chunks, chunkCount, ChunkId::Clock);
	if (myChunk == nullptr)
	{
		return;
	}

	data += myChunk->m_offset;

	ReadAndMove(data, &m_dividerCycleAccumulator, sizeof(uint32_t));
	ReadAndMove(data, &m_timerCycleAccumulator, sizeof(uint32_t));
}
