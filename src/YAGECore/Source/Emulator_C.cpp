#include "../Include/Emulator_C.h"
#include "../Include/Emulator.h"
#include <cstdint>

#ifdef _CINTERFACE

Emulator* FromHandle(EmulatorCHandle handle)
{
	return reinterpret_cast<Emulator*>(handle);
}

EmulatorInputState GetDefaultInputState()
{
    return {0x0F,0x0F};
}

void SetDpadDown(EmulatorInputState* state, EmulatorInputs_DPad pad)
{
	state->m_dPad &= (~static_cast<uint8_t>(pad));
}

void SetButtonDown(EmulatorInputState* state, EmulatorInputs_Buttons button)
{
	state->m_buttons &= (~static_cast<uint8_t>(button));
}

EmulatorCHandle CreateEmulatorHandle()
{
	return reinterpret_cast<EmulatorCHandle>(Emulator::Create());
}

void Delete(EmulatorCHandle emulator)
{
	Emulator* emu = FromHandle(emulator);
	Emulator::Delete(emu);
}

void SetLoggerCallback(EmulatorCHandle emulator, EmulatorLoggerCallback callback)
{
	Emulator* emu = FromHandle(emulator);
	emu->SetLoggerCallback(callback);
}

void Load(EmulatorCHandle emulator, const char* romName, const char* rom, uint32_t size)
{
	Emulator* emu = FromHandle(emulator);
	emu->Load(romName, rom, size);
}

void LoadWithBootrom(EmulatorCHandle emulator, const char* romName, const char* rom, uint32_t size, const char* bootrom, uint32_t bootromSize)
{
	Emulator* emu = FromHandle(emulator);
	emu->Load(romName, rom, size, bootrom, bootromSize);
}

void LoadPersistentMemory(EmulatorCHandle emulator, const char* ram, uint32_t size)
{
	Emulator* emu = FromHandle(emulator);
	emu->LoadPersistentMemory(ram, size);
}

void SetPersistentMemoryCallback(EmulatorCHandle emulator, EmulatorPersistentMemoryCallback callback)
{
	Emulator* emu = FromHandle(emulator);
	emu->SetPersistentMemoryCallback(callback);
}

void SetAudioBuffer(EmulatorCHandle emulator, float* buffer, uint32_t size, uint32_t sampleRate, uint32_t* startOffset)
{
	Emulator* emu = FromHandle(emulator);
	emu->SetAudioBuffer(buffer, size, sampleRate, startOffset);
}

void Step(EmulatorCHandle emulator, EmulatorInputState inputState, double deltaMs)
{
	Emulator* emu = FromHandle(emulator);
	EmulatorInputs::InputState state{ inputState.m_dPad, inputState.m_buttons };
	emu->Step(state, deltaMs);
}

const void* GetFrameBuffer(EmulatorCHandle emulator)
{
	Emulator* emu = FromHandle(emulator);
	return emu->GetFrameBuffer();
}

uint32_t GetNumberOfGeneratedSamples(EmulatorCHandle emulator)
{
	Emulator* emu = FromHandle(emulator);
	return emu->GetNumberOfGeneratedSamples();
}

uint8_t* Serialize(EmulatorCHandle emulator, bool rawData)
{
	Emulator* emu = FromHandle(emulator);
	//HACK, think of something better
	std::vector<uint8_t> serializedData;
	emu->Serialize(rawData, serializedData);

	uint8_t* rawBuffer = new uint8_t[serializedData.size()];
	memcpy(rawBuffer, serializedData.data(), serializedData.size());
	return rawBuffer;
}

void CleanupSerializedMemory(uint8_t* data)
{
	delete[] data;
}

void Deserialize(EmulatorCHandle emulator, const uint8_t* buffer, const uint32_t size)
{
	Emulator* emu = FromHandle(emulator);
	emu->Deserialize(buffer, size);
}

void SetTurboSpeed(EmulatorCHandle emulator, float speed)
{
	Emulator* emu = FromHandle(emulator);
	emu->SetTurboSpeed(speed);
}

#if _DEBUG

void SetInstructionCallback(EmulatorCHandle emulator, uint8_t instr, EmulatorDebugCallback callback, void* userData)
{
	Emulator* emu = FromHandle(emulator);
	emu->SetInstructionCallback(instr, callback, userData);
}

void SetInstructionCountCallback(EmulatorCHandle emulator, uint64_t instr, EmulatorDebugCallback callback, void* userData)
{
	Emulator* emu = FromHandle(emulator);
	emu->SetInstructionCountCallback(instr, callback, userData);
}

void SetPCCallback(EmulatorCHandle emulator, uint16_t pc, EmulatorDebugCallback callback, void* userData)
{
	Emulator* emu = FromHandle(emulator);
	emu->SetPCCallback(pc, callback, userData);
}

void ClearCallbacks(EmulatorCHandle emulator)
{
	Emulator* emu = FromHandle(emulator);
	emu->ClearCallbacks();
}

#endif
#endif