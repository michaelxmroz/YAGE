#include "VirtualMachine.h"

#define ROM_ENTRY_POINT 0x0100

VirtualMachine::VirtualMachine()
	: m_serializer()
	, m_memory(&m_serializer)
	, m_cpu(&m_serializer)
	, m_totalCycles(0)
	, m_joypad()
	, m_clock(&m_serializer)
{
}

VirtualMachine::~VirtualMachine()
{
}

void VirtualMachine::Load(const char* romName, const char* rom, uint32_t size)
{
	m_romName.assign(romName);

	// Setup memory
	m_memory.ClearMemory();
	m_memory.MapROM(&m_serializer, rom, size);
	m_cpu.Reset();
	m_cpu.SetProgramCounter(ROM_ENTRY_POINT);

	Interrupts::Init(m_memory);
	m_clock.Init(m_memory);
	m_ppu.Init(m_memory);
	m_joypad.Init(m_memory);
	m_serial.Init(m_memory);
}

void VirtualMachine::Load(const char* romName, const char* rom, uint32_t size, const char* bootrom, uint32_t bootromSize)
{
	Load(romName, rom, size);
	m_memory.MapBootrom(bootrom, bootromSize);
	m_cpu.SetProgramCounter(0x00);
	m_memory.Write(0xFF40, 0x00);
}

void VirtualMachine::Step(EmulatorInputs::InputState inputState)
{
	bool frameRendered = false;
	while (!frameRendered)
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

void VirtualMachine::LoadPersistentMemory(const char* ram, uint32_t size)
{
	m_memory.MapRAM(ram, size);
}

void VirtualMachine::SetPersistentMemoryCallback(PersistentMemoryCallback callback)
{
	m_memory.RegisterExternalRamDisableCallback(callback);
}

std::vector<uint8_t> VirtualMachine::Serialize() const
{
	return m_serializer.Serialize(m_memory.GetHeaderChecksum(), m_romName);
}
void VirtualMachine::Deserialize(const uint8_t* buffer, const uint32_t size)
{
	m_serializer.Deserialize(buffer, size, m_memory.GetHeaderChecksum());
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
