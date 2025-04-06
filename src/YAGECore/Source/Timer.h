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
	static void WriteTIMA(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue, void* userData);
	static void WriteTMA(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue, void* userData);
	static void WriteTAC(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue, void* userData);

private:
	void CheckForTimerTick(uint16_t timerBits, bool isTimerEnabled, Memory& memory);
	void TickTimer(Memory& memory);

	void Serialize(uint8_t* data) override;
	void Deserialize(const uint8_t* data) override;
	virtual uint32_t GetSerializationSize() override;

	enum class TIMAReloadState : uint8_t
	{
		None,
		Overflowed,
		InterruptTriggered
	};

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
	TIMAReloadState m_TIMAReloadState;
};

