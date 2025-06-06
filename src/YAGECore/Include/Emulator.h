#pragma once

#include "Emulator_C.h"

typedef void* (*YAGEAllocFunc)(uint32_t);
typedef void (*YAGEFreeFunc)(void*);

namespace EmulatorInputs
{
	enum class DPad
	{
		Right = 1,
		Left = 2,
		Up = 4,
		Down = 8
	};

	enum class Buttons
	{
		A = 1,
		B = 2,
		Select = 4,
		Start = 8
	};

	struct InputState
	{
		InputState() : m_dPad(0x0F), m_buttons(0x0F)
		{
		}

		InputState(uint8_t dPad, uint8_t buttons) : m_dPad(dPad), m_buttons(buttons)
		{
		}

		uint8_t m_dPad;
		uint8_t m_buttons;

		void SetButtonDown(DPad pad)
		{
			m_dPad &= (~static_cast<uint8_t>(pad));
		}
		void SetButtonDown(Buttons button)
		{
			m_buttons &= (~static_cast<uint8_t>(button));
		}
	};
}

namespace EmulatorConstants
{
	const uint32_t SCREEN_WIDTH = EMULATOR_SCREEN_WIDTH;
	const uint32_t SCREEN_HEIGHT = EMULATOR_SCREEN_HEIGHT;
	const uint32_t SCREEN_SIZE = EMULATOR_SCREEN_SIZE;
	const double PREFERRED_REFRESH_RATE = EMULATOR_PREFERRED_REFRESH_RATE;
}

class Emulator
{
public:

	typedef void (*LoggerCallback)(const char* message, uint8_t severity);
	typedef void (*PersistentMemoryCallback)(const void* data, uint32_t size);
#if _DEBUG
	typedef void (*DebugCallback)(void* userData);
#endif

	static Emulator* Create(YAGEAllocFunc allocFunc, YAGEFreeFunc freeFunc);
	static void Delete(Emulator* emulator);

	virtual void SetLoggerCallback(LoggerCallback callback) = 0;
	virtual void Load(const char* romName, const char* rom, uint32_t size) = 0;
	virtual void Load(const char* romName, const char* rom, uint32_t size, const char* bootrom, uint32_t bootromSize) = 0;
	virtual void LoadPersistentMemory(const char* ram, uint32_t size) = 0;
	virtual void SetPersistentMemoryCallback(PersistentMemoryCallback callback) = 0;

	virtual void SetAudioBuffer(float* buffer, uint32_t size, uint32_t sampleRate, uint32_t* startOffset) = 0;

	virtual void Step(EmulatorInputs::InputState, double deltaMs, bool microStepping) = 0;
	virtual const void* GetFrameBuffer() = 0;
	virtual uint32_t GetNumberOfGeneratedSamples() = 0;

	virtual SerializationView Serialize(bool rawData) = 0;
	virtual void Deserialize(const SerializationView& data) = 0;

	virtual void SetTurboSpeed(float speed) = 0;

	uint32_t GetMemoryUse() const;

#if _DEBUG

#define MAX_MNEMONIC_LENGTH 10

	struct CPUState
	{
		uint8_t m_regA{ 0 };
		uint8_t m_regF{ 0 };
		uint8_t m_regB{ 0 };
		uint8_t m_regC{ 0 };
		uint8_t m_regD{ 0 };
		uint8_t m_regE{ 0 };
		uint8_t m_regH{ 0 };
		uint8_t m_regL{ 0 };
		uint16_t m_regPC{ 0 };
		uint16_t m_regSP{ 0 };
		bool m_running{ true };
		bool m_halted{ false };
		char m_currentInstruction[MAX_MNEMONIC_LENGTH]{};
		int m_instructionDurationCycles{ 0 };
		int m_cyclesProcessed{ 0 };
		bool m_handlingInterrupt{ false };
		uint32_t m_tCyclesStepped{ 0 };
	};

	struct PPUState
	{
		uint8_t m_mode{ 0 };
		uint32_t m_totalCycles{ 0 };
		int32_t m_cycleDebt{ 0 };
		uint8_t m_lineY{ 0 };
		uint8_t m_lineX{ 0 };
		uint8_t m_lineSpriteCount{ 0 };
		uint32_t m_cyclesInLine{ 0 };
		int32_t m_cyclesInMode{ 0 };
	};

	virtual void SetInstructionCallback(uint8_t instr, Emulator::DebugCallback callback, void* userData) = 0;
	virtual void SetInstructionCountCallback(uint64_t instr, Emulator::DebugCallback callback, void* userData) = 0;
	virtual void SetPCCallback(uint16_t pc, Emulator::DebugCallback callback, void* userData) = 0;
	virtual void SetDataCallback(uint16_t addr, Emulator::DebugCallback callback, void* userData) = 0;
	virtual void ClearCallbacks() = 0;
	virtual CPUState GetCPUState() = 0;
	virtual PPUState GetPPUState() = 0;
	virtual void* GetRawMemoryView() = 0;


#endif
	virtual ~Emulator();
};