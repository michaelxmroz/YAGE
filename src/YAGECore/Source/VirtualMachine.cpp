#include "VirtualMachine.h"

#define ROM_ENTRY_POINT 0x0100

VirtualMachine::VirtualMachine()
	: m_serializer()
	, m_memory(&m_serializer)
	, m_cpu(&m_serializer)
	, m_totalCycles(0)
	, m_joypad()
	, m_clock(&m_serializer)
	, m_frameRendered(false)
	, m_stepDuration(0.0)
	, m_ppu(&m_serializer)
	, m_apu(&m_serializer)
	, m_samplesGenerated(0)
	, m_turbospeed(1)
	, m_serial(&m_serializer)
	, m_tCyclesStepped(0)
{
}

void VirtualMachine::Load(const char* romName, const char* rom, uint32_t size)
{
	m_romName.Assign(romName);

	// Setup memory
	m_memory.ClearMemory();
	m_memory.MapROM(&m_serializer, rom, size);
	m_cpu.ResetToBootromValues();
	m_cpu.SetProgramCounter(ROM_ENTRY_POINT);

	Interrupts::Init(m_memory);
	m_clock.Init(m_memory);
	m_ppu.Init(m_memory);
	m_apu.Init(m_memory);
	m_joypad.Init(m_memory);
	m_serial.Init(m_memory);

#if _DEBUG
	m_cpu.DisassembleROM(m_memory);
#endif
}

void VirtualMachine::Load(const char* romName, const char* rom, uint32_t size, const char* bootrom, uint32_t bootromSize)
{
	Load(romName, rom, size);
	m_cpu.Reset();
	m_memory.MapBootrom(bootrom, bootromSize);
	m_cpu.SetProgramCounter(0x00);
	m_memory.Write(0xFF40, 0x00);

#if _DEBUG
	m_cpu.DisassembleROM(m_memory);
#endif
}

void VirtualMachine::SetAudioBuffer(float* buffer, uint32_t size, uint32_t sampleRate, uint32_t* startOffset)
{
	m_apu.SetExternalAudioBuffer(buffer, size, sampleRate, startOffset);
}

void VirtualMachine::Step(EmulatorInputs::InputState inputState, double deltaMs, bool microStepping)
{
	m_totalCycles = 0;
	m_samplesGenerated = 0;
	while (m_stepDuration < deltaMs)
	{
		bool tCycleStep = false;
		uint32_t cyclesPassed = microStepping ? 1 : MCYCLES_TO_CYCLES; // step either 1 or 4 tcycles. 
		m_tCyclesStepped += cyclesPassed;
		if (m_tCyclesStepped >= MCYCLES_TO_CYCLES)
		{
			m_tCyclesStepped = 0;
			tCycleStep = true;
		}

		if (tCycleStep)
		{
			m_memory.Update();
			m_joypad.Update(inputState, m_memory);
			m_clock.Increment(MCYCLES_TO_CYCLES, m_memory);
		}

		m_ppu.Render(cyclesPassed, m_memory);

		bool shouldBreak = false;
		if (tCycleStep)
		{
			m_samplesGenerated += m_apu.Update(m_memory, MCYCLES_TO_CYCLES, m_turbospeed);
			m_serial.Update(m_memory, 1);
			shouldBreak = m_cpu.Step(m_memory);
		}

		m_totalCycles += cyclesPassed;
		double cycleDurationS = static_cast<double>((cyclesPassed)) / (static_cast<double>(CPU_FREQUENCY) * static_cast<double>(m_turbospeed));
		m_stepDuration += cycleDurationS * 1000.0;

		if (shouldBreak)
		{
			break;
		}
	}
	m_stepDuration -= deltaMs;
}

const void* VirtualMachine::GetFrameBuffer()
{
	return m_ppu.GetFrameBuffer();
}

uint32_t VirtualMachine::GetNumberOfGeneratedSamples()
{
	return m_samplesGenerated;
}

void VirtualMachine::SetLoggerCallback(LoggerCallback callback)
{
	Logger_Helpers::loggerCallback = callback;
}

void VirtualMachine::LoadPersistentMemory(const char* ram, uint32_t size)
{
	m_memory.DeserializePersistentData(ram, size);
}

void VirtualMachine::SetPersistentMemoryCallback(PersistentMemoryCallback callback)
{
	m_memory.RegisterRamSaveCallback(callback);
}

SerializationView VirtualMachine::Serialize(bool rawData)
{
	return m_serializer.Serialize(m_memory.GetHeaderChecksum(), m_romName, rawData);
}
void VirtualMachine::Deserialize(const SerializationView& data)
{
	m_serializer.Deserialize(data, m_memory.GetHeaderChecksum());
#if _DEBUG
	m_cpu.DisassembleROM(m_memory);
#endif
}

void VirtualMachine::SetTurboSpeed(float speed)
{
	m_turbospeed = speed;
}

#if _DEBUG

void VirtualMachine::SetInstructionCallback(uint8_t instr, Emulator::DebugCallback callback, void* userData)
{
	m_cpu.SetInstructionCallback(instr, callback, userData);
}

void VirtualMachine::SetInstructionCountCallback(uint64_t instr, Emulator::DebugCallback callback, void* userData)
{
	m_cpu.SetInstructionCountCallback(instr, callback, userData);
}

void VirtualMachine::SetPCCallback(uint16_t pc, Emulator::DebugCallback callback, void* userData)
{
	m_cpu.SetPCCallback(pc, callback, userData);
}

void VirtualMachine::SetDataCallback(uint16_t addr, Emulator::DebugCallback callback, void* userData)
{
	m_memory.SetMemoryCallback(addr, callback, userData);
}

void VirtualMachine::ClearCallbacks()
{
	m_cpu.ClearCallbacks();
	m_memory.ClearCallbacks();
}

Emulator::CPUState VirtualMachine::GetCPUState()
{
	auto cpuState = m_cpu.GetCPUState();
	cpuState.m_tCyclesStepped = m_tCyclesStepped;
	return cpuState;
}

Emulator::PPUState VirtualMachine::GetPPUState()
{
	return m_ppu.GetPPUState();
}

void* VirtualMachine::GetRawMemoryView()
{
	return m_memory.GetRawMemoryView();
}

Emulator::DisassemblyInfo VirtualMachine::GetDisassemblyInfo(uint16_t addr)
{
	return m_cpu.GetDisassemblyInfo(addr, m_memory);
}

#endif

#ifdef _TESTING
void VirtualMachine::StopOnInstruction(uint8_t instr)
{
	m_cpu.StopOnInstruction(instr);
}

bool VirtualMachine::HasReachedInstruction(uint8_t instr)
{
	return m_cpu.HasReachedInstruction(instr);
}

Registers& VirtualMachine::GetRegisters()
{
	return m_cpu.GetRegisters();
}
#endif
