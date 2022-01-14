#include "VirtualMachine.h"

#define ROM_ENTRY_POINT 0x0100

VirtualMachine::VirtualMachine()
	: m_memory()
	, m_cpu()
	, m_totalCycles(0)
	, m_renderCallback(nullptr)
	, m_joypad()
{
}

void VirtualMachine::SetRenderCallback(RenderFunc callback)
{
	m_renderCallback = callback;
}

bool VirtualMachine::Load(std::shared_ptr<std::vector<char>> romBlob)
{
	m_romBlob = romBlob;

	return true;
}

bool VirtualMachine::Start()
{
	// Setup memory
	m_memory.ClearMemory();
	m_memory.MapROM(m_romBlob.get());
	m_cpu.Reset();
	m_cpu.SetProgramCounter(ROM_ENTRY_POINT);

	Interrupts::Init(m_memory);
	m_clock.Init(m_memory);
	m_ppu.Init(m_memory, m_renderCallback);
	m_joypad.Init(m_memory);

	// Run
	while (true)
	{
		uint32_t cyclesPassed = m_cpu.Step(m_memory);

		m_clock.Increment(cyclesPassed, m_memory);

		m_ppu.Render(cyclesPassed, m_memory);

		m_joypad.Update(m_memory);
		//TODO process APU
		//TODO render
		//TODO audio
		m_totalCycles += cyclesPassed;
	}

	return true;
}


