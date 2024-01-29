#pragma once
#include <cstdint>
#include "Memory.h"

class Clock : ISerializable
{
public:
	Clock();
	explicit Clock(Serializer* serializer);

	void Init(Memory& memory);

	void Increment(uint32_t mCycles, Memory& memory);

	void Reset();
	static void ResetDivider(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue, void* userData);
private:
	virtual void Serialize(std::vector<Chunk>& chunks, std::vector<uint8_t>& data) override;
	virtual void Deserialize(const Chunk* chunks, const uint32_t& chunkCount, const uint8_t* data, const uint32_t& dataSize) override;

	uint32_t m_dividerCycleAccumulator;
	uint32_t m_timerCycleAccumulator;
};

