#pragma once
#include "../Include/Emulator.h"
#include "Registers.h"
#include "CPU.h"
#include "Memory.h"
#include "Clock.h"
#include "PPU.h"
#include "Joypad.h"

class VirtualMachine : public Emulator
{
public:
	VirtualMachine();
	virtual ~VirtualMachine() override;

	virtual void Load(const char* rom, uint32_t size) override;
	virtual void Step(EmulatorInputs::InputState) override;
	virtual const void* GetFrameBuffer() override;
	virtual void SetLoggerCallback(LoggerCallback callback) override;
private:
	Memory m_memory;
	CPU m_cpu;
	Clock m_clock;
	PPU m_ppu;
	Joypad m_joypad;

	uint64_t m_totalCycles;
};

