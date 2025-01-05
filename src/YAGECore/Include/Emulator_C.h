#pragma once

#ifdef _CPP
extern "C"
{
#else
#ifndef _CINTERFACE
#define _CINTERFACE
#endif
#endif

#ifdef _CINTERFACE
#include <stdint.h>
#endif


#define EMULATOR_SCREEN_WIDTH 160
#define EMULATOR_SCREEN_HEIGHT 144
#define EMULATOR_SCREEN_SIZE EMULATOR_SCREEN_WIDTH * EMULATOR_SCREEN_HEIGHT
#define EMULATOR_PREFERRED_REFRESH_RATE 59.73

#ifdef _CINTERFACE

	typedef void (*EmulatorLoggerCallback)(const char* message, uint8_t severity);
	typedef void (*EmulatorPersistentMemoryCallback)(const void* data, uint32_t size);
#if _DEBUG
	typedef void (*EmulatorDebugCallback)(void* userData);
#endif

	typedef void* (*YAGEAllocFunc)(uint32_t);
	typedef void (*YAGEFreeFunc)(void*);

	enum EmulatorInputs_DPad
	{
		Right = 1,
		Left = 2,
		Up = 4,
		Down = 8
	};

	enum EmulatorInputs_Buttons
	{
		A = 1,
		B = 2,
		Select = 4,
		Start = 8
	};

	struct EmulatorInputState
	{
		uint8_t m_dPad;
		uint8_t m_buttons;
	};

	typedef enum EmulatorInputs_DPad EmulatorInputs_DPad;
	typedef enum EmulatorInputs_Buttons EmulatorInputs_Buttons;
	typedef struct EmulatorInputState EmulatorInputState;

	EmulatorInputState GetDefaultInputState();

	void SetDpadDown(EmulatorInputState* state, EmulatorInputs_DPad pad);

	void SetButtonDown(EmulatorInputState* state, EmulatorInputs_Buttons button);

	struct EmulatorC;
	typedef struct EmulatorC* EmulatorCHandle;

	EmulatorCHandle CreateEmulatorHandle(YAGEAllocFunc allocFunc, YAGEFreeFunc freeFunc);
	void Delete(EmulatorCHandle emulator);

	void SetLoggerCallback(EmulatorCHandle emulator, EmulatorLoggerCallback callback);
	void Load(EmulatorCHandle emulator, const char* romName, const char* rom, uint32_t size);
	void LoadWithBootrom(EmulatorCHandle emulator, const char* romName, const char* rom, uint32_t size, const char* bootrom, uint32_t bootromSize);
	void LoadPersistentMemory(EmulatorCHandle emulator, const char* ram, uint32_t size);
	void SetPersistentMemoryCallback(EmulatorCHandle emulator, EmulatorPersistentMemoryCallback callback);

	void SetAudioBuffer(EmulatorCHandle emulator, float* buffer, uint32_t size, uint32_t sampleRate, uint32_t* startOffset);

	void Step(EmulatorCHandle emulator, EmulatorInputState inputState, double deltaMs);
	const void* GetFrameBuffer(EmulatorCHandle emulator);
	uint32_t GetNumberOfGeneratedSamples(EmulatorCHandle emulator);

	SerializationView Serialize(EmulatorCHandle emulator, uint8_t rawData);
	void Deserialize(EmulatorCHandle emulator, const SerializationView& data);

	void SetTurboSpeed(EmulatorCHandle emulator, float speed);

#if _DEBUG
	void SetInstructionCallback(EmulatorCHandle emulator, uint8_t instr, EmulatorDebugCallback callback, void* userData);
	void SetInstructionCountCallback(EmulatorCHandle emulator, uint64_t instr, EmulatorDebugCallback callback, void* userData);
	void SetPCCallback(EmulatorCHandle emulator, uint16_t pc, EmulatorDebugCallback callback, void* userData);
	void ClearCallbacks(EmulatorCHandle emulator);
#endif

#endif

#ifdef _CPP
}
#endif