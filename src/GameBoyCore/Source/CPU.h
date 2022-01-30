#pragma once
#include "Memory.h"
#include "Registers.h"
#include "InstructionFunctions.h"
#include "Interrupts.h"

#define INSTRUCTION_SET_SIZE 512

class CPU
{
public:
	CPU(bool enableInterruptHandling = true);

#if _DEBUG
	void StopOnInstruction(uint8_t instr);
	bool HasReachedInstruction(Memory& memory);
#endif
	uint32_t Step(Memory& memory);

	void SetProgramCounter(unsigned short addr);

	void Reset();

	// For tests
//#ifdef _DEBUG

	uint32_t Step(uint8_t* memory)
	{
		Memory mem(memory);
		return Step(mem);
	}

	Registers& GetRegisters()
	{
		return m_registers;
	}
//#endif // _DEBUG

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

	Registers m_registers;

	const Instruction m_instructions[INSTRUCTION_SET_SIZE];

	bool m_haltBug;
	bool m_delayedInterruptHandling;
	const bool m_InterruptHandlingEnabled;
#if _DEBUG
	uint8_t m_stopOnInstruction;
	bool m_stopOnInstructionEnabled;
#endif
};

