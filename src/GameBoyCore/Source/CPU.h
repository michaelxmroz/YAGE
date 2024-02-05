#pragma once
#include "Memory.h"
#include "Registers.h"
#include "InstructionFunctions.h"
#include "Interrupts.h"
#include <map>

#define INSTRUCTION_SET_SIZE 512

class CPU : ISerializable
{
public:
	CPU();
	explicit CPU(bool enableInterruptHandling);
	explicit CPU(Serializer* serializer);
	CPU(Serializer* serializer, bool enableInterruptHandling);
	~CPU();

#if _DEBUG
	void SetInstructionCallback(uint8_t instr, Emulator::DebugCallback callback);
	void SetInstructionCountCallback(uint64_t instrCount, Emulator::DebugCallback callback);
	void SetPCCallback(uint16_t pc, Emulator::DebugCallback callback);
	void ClearCallbacks();

#endif
	uint32_t Step(Memory& memory);

	void SetProgramCounter(unsigned short addr);

	void Reset();
	void ResetToBootromValues();

	// For tests
#ifdef _DEBUG

	uint32_t Step(uint8_t* memory)
	{
		Memory mem(memory);
		return Step(mem);
	}

	Registers& GetRegisters()
	{
		return m_registers;
	}
#endif // _DEBUG

private:
	typedef uint32_t (*InstructionFunc)(const char* mnemonic, Registers* registers, Memory& memory);

	struct Instruction
	{
		const char* m_mnemonic;
		const uint8_t m_length;
		const uint8_t m_duration;
		const InstructionFunc m_func;
	};

	void ClearRegisters();
	void ExecuteInstruction(Memory& memory, uint32_t& mCycles);
	void ProcessInterrupts(Memory& memory, uint32_t& mCycles);

	virtual void Serialize(std::vector<Chunk>& chunks, std::vector<uint8_t>& data) override;
	virtual void Deserialize(const Chunk* chunks, const uint32_t& chunkCount, const uint8_t* data, const uint32_t& dataSize) override;

	Registers m_registers;

	const Instruction m_instructions[INSTRUCTION_SET_SIZE];

	bool m_haltBug;
	bool m_delayedInterruptHandling;
	const bool m_InterruptHandlingEnabled;

#if _DEBUG

	std::map<uint16_t, Emulator::DebugCallback> DEBUG_PCCallbackMap;
	std::map<uint8_t, Emulator::DebugCallback> DEBUG_instrCallbackMap;
	std::map<uint64_t, Emulator::DebugCallback> DEBUG_instrCountCallbackMap;

	uint64_t DEBUG_instructionCount = 0;

	char* DEBUG_CPUInstructionLog;
	const char* DEBUG_LogTemplate = "A:00 F:00 B:00 C:00 D:00 E:00 H:00 L:00 SP:0000 PC:0000 PCMEM:00,00,00,00\n";
#endif
};

