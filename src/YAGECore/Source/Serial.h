#pragma once
#include "Memory.h"

enum class SerialMode : uint8_t
{
	None = 0,
	Loopback
};

//Loopback serial connector
class Serial : ISerializable
{
public:
	Serial(GamestateSerializer* serializer);
	void Init(Memory& memory);
	void Update(Memory& memory, uint32_t mCycles);
private:

	void Serialize(uint8_t* data) override;
	void Deserialize(const uint8_t* data) override;
	virtual uint32_t GetSerializationSize() override;

	static void ResetClock(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue, void* userData);
	void TransferNextBit(Memory& memory);

	uint32_t m_accumulatedCycles;
	uint32_t m_bitsTransferred;
	SerialMode m_mode;
};

