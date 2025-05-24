#pragma once
#include "../Include/Emulator.h"
#include "YString.h"
#include "Registers.h"
#include "CPU.h"
#include "Memory.h"
#include "Timer.h"
#include "PPU.h"
#include "Joypad.h"
#include "Serial.h"
#include "APU.h"

class VirtualMachine : public Emulator
{
public:
	VirtualMachine();
	virtual ~VirtualMachine() override = default;

	virtual void Load(const char* romName, const char* rom, uint32_t size) override;
	virtual void Load(const char* romName, const char* rom, uint32_t size, const char* bootrom, uint32_t bootromSize) override;

	virtual void LoadPersistentMemory(const char* ram, uint32_t size) override;
	virtual void SetPersistentMemoryCallback(PersistentMemoryCallback callback) override;

	void SetAudioBuffer(float* buffer, uint32_t size, uint32_t sampleRate, uint32_t* startOffset) override;

	virtual void Step(EmulatorInputs::InputState, double deltaMs) override;
	virtual const void* GetFrameBuffer() override;
	uint32_t GetNumberOfGeneratedSamples() override;

	virtual void SetLoggerCallback(LoggerCallback callback) override;

	virtual SerializationView Serialize(bool rawData) override;
	virtual void Deserialize(const SerializationView& data) override;

	virtual void SetTurboSpeed(float speed) override;

#if _DEBUG
	virtual void SetInstructionCallback(uint8_t instr, Emulator::DebugCallback callback, void* userData) override;
	virtual void SetInstructionCountCallback(uint64_t instr, Emulator::DebugCallback callback, void* userData) override;
	virtual void SetPCCallback(uint16_t pc, Emulator::DebugCallback callback, void* userData) override;
	virtual void SetDataCallback(uint16_t addr, Emulator::DebugCallback callback, void* userData) override;
	virtual void ClearCallbacks() override;
	virtual CPUState GetCPUState() override;
	virtual PPUState GetPPUState() override;
	virtual void* GetRawMemoryView() override;
#endif

#if _TESTING
	void StopOnInstruction(uint8_t instr);
	bool HasReachedInstruction(uint8_t instr);
	Registers& GetRegisters();
#endif
private:
	GamestateSerializer m_serializer;
	Memory m_memory;
	CPU m_cpu;
	Timer m_clock;
	PPU m_ppu;
	APU m_apu;
	Joypad m_joypad;
	Serial m_serial;

	yString m_romName;
	uint64_t m_totalCycles;
	uint32_t m_samplesGenerated;
	bool m_frameRendered;
	double m_stepDuration;
	float m_turbospeed;

};

