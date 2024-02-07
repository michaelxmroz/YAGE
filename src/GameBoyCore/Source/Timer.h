#pragma once
#include <cstdint>
#include "Memory.h"

class Timer : ISerializable
{
public:
	Timer();
	explicit Timer(Serializer* serializer);

	void Init(Memory& memory);

	void Increment(uint32_t mCycles, Memory& memory);

	void Reset();
	static void ResetDivider(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue, void* userData);
private:
	virtual void Serialize(std::vector<Chunk>& chunks, std::vector<uint8_t>& data) override;
	virtual void Deserialize(const Chunk* chunks, const uint32_t& chunkCount, const uint8_t* data, const uint32_t& dataSize) override;

	union // Div timer
	{
		uint16_t m_divTotal;
		struct
		{
			uint8_t m_divLow;
			uint8_t m_divHigh;
		};
	};
	bool m_previousCycleTimerModuloEdge;
	uint8_t m_currentCyclesStepped;
};

