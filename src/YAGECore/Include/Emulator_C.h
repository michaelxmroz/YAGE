#pragma once

#ifdef FREESTANDING

#define _CINTERFACE

// Signed integer types
using int8_t = __INT8_TYPE__;
using int16_t = __INT16_TYPE__;
using int32_t = __INT32_TYPE__;
using int64_t = __INT64_TYPE__;

// Unsigned integer types
using uint8_t = __UINT8_TYPE__;
using uint16_t = __UINT16_TYPE__;
using uint32_t = __UINT32_TYPE__;
using uint64_t = __UINT64_TYPE__;

// Pointer-sized integer types
using intptr_t = __INTPTR_TYPE__;
using uintptr_t = __UINTPTR_TYPE__;

// Maximum-width integer types
using intmax_t = __INTMAX_TYPE__;
using uintmax_t = __UINTMAX_TYPE__;

typedef uint64_t size_t;

extern "C"
{
#endif

#ifdef _CINTERFACE
#include <stdint.h>
#else
#include <cstdint>
#endif

struct SerializationView
{
	uint8_t* data;
	uint32_t size;
};

#define EMULATOR_SCREEN_WIDTH 160
#define EMULATOR_SCREEN_HEIGHT 144
#define EMULATOR_SCREEN_SIZE EMULATOR_SCREEN_WIDTH * EMULATOR_SCREEN_HEIGHT
#define EMULATOR_PREFERRED_REFRESH_RATE 59.73
#define EMULATOR_CLOCK_MS 0.0002384185791015625
#define EMULATOR_GB_MEMORY_SIZE 0x10000

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

	struct SerializationView Serialize(EmulatorCHandle emulator, uint8_t rawData);
	void Deserialize(EmulatorCHandle emulator, const struct SerializationView* data);

	void SetTurboSpeed(EmulatorCHandle emulator, float speed);

#if _DEBUG
	void SetInstructionCallback(EmulatorCHandle emulator, uint8_t instr, EmulatorDebugCallback callback, void* userData);
	void SetInstructionCountCallback(EmulatorCHandle emulator, uint64_t instr, EmulatorDebugCallback callback, void* userData);
	void SetPCCallback(EmulatorCHandle emulator, uint16_t pc, EmulatorDebugCallback callback, void* userData);
	void ClearCallbacks(EmulatorCHandle emulator);
#endif

#endif

#ifdef FREESTANDING
}
#endif