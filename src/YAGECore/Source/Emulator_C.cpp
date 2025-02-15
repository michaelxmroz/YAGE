#include "../Include/Emulator_C.h"

#include "../Include/Emulator.h"
#include "CppIncludes.h"
#include "Allocator.h"

#ifdef FREESTANDING
void __cxa_pure_virtual()
{
	while (1);
}
#endif

#ifdef _CINTERFACE

extern "C" inline Emulator* FromHandle(EmulatorCHandle handle)
{
	return reinterpret_cast<Emulator*>(handle);
}

extern "C" EmulatorInputState GetDefaultInputState()
{
    return {0x0F,0x0F};
}

extern "C" void SetDpadDown(EmulatorInputState* state, EmulatorInputs_DPad pad)
{
	state->m_dPad &= (~static_cast<uint8_t>(pad));
}

extern "C" void SetButtonDown(EmulatorInputState* state, EmulatorInputs_Buttons button)
{
	state->m_buttons &= (~static_cast<uint8_t>(button));
}

extern "C" EmulatorCHandle CreateEmulatorHandle(YAGEAllocFunc allocFunc, YAGEFreeFunc freeFunc)
{
	return reinterpret_cast<EmulatorCHandle>(Emulator::Create(allocFunc, freeFunc));
}

extern "C" void Delete(EmulatorCHandle emulator)
{
	Emulator* emu = FromHandle(emulator);
	Emulator::Delete(emu);
}

extern "C" void SetLoggerCallback(EmulatorCHandle emulator, EmulatorLoggerCallback callback)
{
	Emulator* emu = FromHandle(emulator);
	emu->SetLoggerCallback(callback);
}

extern "C" void Load(EmulatorCHandle emulator, const char* romName, const char* rom, uint32_t size)
{
	Emulator* emu = FromHandle(emulator);
	emu->Load(romName, rom, size);
}

extern "C" void LoadWithBootrom(EmulatorCHandle emulator, const char* romName, const char* rom, uint32_t size, const char* bootrom, uint32_t bootromSize)
{
	Emulator* emu = FromHandle(emulator);
	emu->Load(romName, rom, size, bootrom, bootromSize);
}

extern "C" void LoadPersistentMemory(EmulatorCHandle emulator, const char* ram, uint32_t size)
{
	Emulator* emu = FromHandle(emulator);
	emu->LoadPersistentMemory(ram, size);
}

extern "C" void SetPersistentMemoryCallback(EmulatorCHandle emulator, EmulatorPersistentMemoryCallback callback)
{
	Emulator* emu = FromHandle(emulator);
	emu->SetPersistentMemoryCallback(callback);
}

extern "C" void SetAudioBuffer(EmulatorCHandle emulator, float* buffer, uint32_t size, uint32_t sampleRate, uint32_t* startOffset)
{
	Emulator* emu = FromHandle(emulator);
	emu->SetAudioBuffer(buffer, size, sampleRate, startOffset);
}

extern "C" void Step(EmulatorCHandle emulator, EmulatorInputState inputState, double deltaMs)
{
	Emulator* emu = FromHandle(emulator);
	EmulatorInputs::InputState state{ inputState.m_dPad, inputState.m_buttons };
	emu->Step(state, deltaMs);
}

extern "C" const void* GetFrameBuffer(EmulatorCHandle emulator)
{
	Emulator* emu = FromHandle(emulator);
	return emu->GetFrameBuffer();
}

extern "C" uint32_t GetNumberOfGeneratedSamples(EmulatorCHandle emulator)
{
	Emulator* emu = FromHandle(emulator);
	return emu->GetNumberOfGeneratedSamples();
}

extern "C" SerializationView Serialize(EmulatorCHandle emulator, uint8_t rawData)
{
	Emulator* emu = FromHandle(emulator);
	return emu->Serialize(rawData > 0);
}

extern "C" void Deserialize(EmulatorCHandle emulator, const SerializationView* buffer)
{
	Emulator* emu = FromHandle(emulator);
	emu->Deserialize(*buffer);
}

extern "C" void SetTurboSpeed(EmulatorCHandle emulator, float speed)
{
	Emulator* emu = FromHandle(emulator);
	emu->SetTurboSpeed(speed);
}

#if _DEBUG

extern "C" void SetInstructionCallback(EmulatorCHandle emulator, uint8_t instr, EmulatorDebugCallback callback, void* userData)
{
	Emulator* emu = FromHandle(emulator);
	emu->SetInstructionCallback(instr, callback, userData);
}

extern "C" void SetInstructionCountCallback(EmulatorCHandle emulator, uint64_t instr, EmulatorDebugCallback callback, void* userData)
{
	Emulator* emu = FromHandle(emulator);
	emu->SetInstructionCountCallback(instr, callback, userData);
}

extern "C" void SetPCCallback(EmulatorCHandle emulator, uint16_t pc, EmulatorDebugCallback callback, void* userData)
{
	Emulator* emu = FromHandle(emulator);
	emu->SetPCCallback(pc, callback, userData);
}

extern "C" void ClearCallbacks(EmulatorCHandle emulator)
{
	Emulator* emu = FromHandle(emulator);
	emu->ClearCallbacks();
}

#endif
#endif