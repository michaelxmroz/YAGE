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

	virtual void Step(EmulatorInputs::InputState, double deltaMs) = 0;
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
		uint8_t regA;                  // Accumulator
		uint8_t regF;                  // Flags register
		uint8_t regB;
		uint8_t regC;
		uint8_t regD;
		uint8_t regE;
		uint8_t regH;
		uint8_t regL;
		uint16_t regPC;                // Program Counter
		uint16_t regSP;                // Stack Pointer
		bool running = true;           // CPU running state
		bool halted = false;           // Halt flag
		char currentInstruction[MAX_MNEMONIC_LENGTH];     // Mnemonic of the current instruction
		int instructionDurationCycles = 0;  // Total cycles this instruction lasts
		int cyclesProcessed = 0;            // Cycles processed so far
		bool handlingInterrupt = false;     // If in interrupt handling
	};

	virtual void SetInstructionCallback(uint8_t instr, Emulator::DebugCallback callback, void* userData) = 0;
	virtual void SetInstructionCountCallback(uint64_t instr, Emulator::DebugCallback callback, void* userData) = 0;
	virtual void SetPCCallback(uint16_t pc, Emulator::DebugCallback callback, void* userData) = 0;
	virtual void SetDataCallback(uint16_t addr, Emulator::DebugCallback callback, void* userData) = 0;
	virtual void ClearCallbacks() = 0;
	virtual CPUState GetCPUState() = 0;
	virtual void* GetRawMemoryView() = 0;


#endif
	virtual ~Emulator();
};