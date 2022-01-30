#include "VirtualMachine.h"

#define ROM_ENTRY_POINT 0x0100

VirtualMachine::VirtualMachine()
	: m_memory()
	, m_cpu()
	, m_totalCycles(0)
	, m_joypad()
{
}

VirtualMachine::~VirtualMachine()
{
}

void VirtualMachine::Load(const char* rom, uint32_t size)
{
	// Setup memory
	m_memory.ClearMemory();
	m_memory.MapROM(rom, size);
	m_cpu.Reset();
	m_cpu.SetProgramCounter(ROM_ENTRY_POINT);

	Interrupts::Init(m_memory);
	m_clock.Init(m_memory);
	m_ppu.Init(m_memory);
	m_joypad.Init(m_memory);
	m_serial.Init(m_memory);
}

void VirtualMachine::Step(EmulatorInputs::InputState inputState)
{
	bool frameRendered = false;
	while (!frameRendered && m_cpu.GetRegisters().CpuState == Registers::State::Running)
	{
		m_joypad.Update(inputState, m_memory);
		uint32_t cyclesPassed = m_cpu.Step(m_memory);
		m_clock.Increment(cyclesPassed, m_memory);
		frameRendered = m_ppu.Render(cyclesPassed, m_memory);

		//TODO process APU
		m_totalCycles += cyclesPassed;
	}
}

const void* VirtualMachine::GetFrameBuffer()
{
	return m_ppu.GetFrameBuffer();
}

void VirtualMachine::SetLoggerCallback(LoggerCallback callback)
{
	Logger_Helpers::loggerCallback = callback;
}

#if _DEBUG
void VirtualMachine::StopOnInstruction(uint8_t instr)
{
	m_cpu.StopOnInstruction(instr);
}

bool VirtualMachine::HasReachedInstruction()
{
	return m_cpu.HasReachedInstruction(m_memory);
}
Registers& VirtualMachine::GetRegisters()
{
	return m_cpu.GetRegisters();
}
#endif