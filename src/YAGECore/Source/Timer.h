#pragma once
#include "CppIncludes.h"
#include "Memory.h"

class Timer : ISerializable
{
public:
	Timer();
	explicit Timer(GamestateSerializer* serializer);

	void Init(Memory& memory);

	void Increment(uint32_t mCycles, Memory& memory);

	void Reset();
	static void ResetDivider(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue, void* userData);
private:
	void Serialize(uint8_t* data) override;
	void Deserialize(const uint8_t* data) override;
	virtual uint32_t GetSerializationSize() override;

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
};

