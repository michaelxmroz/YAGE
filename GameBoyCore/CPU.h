#pragma once
#include "Registers.h"
#include "InstructionFunctions.h"

#define DEFAULT_STACK_POINTER 0xFFFE
#define INSTRUCTION_SET_SIZE 512
#define EXTENSION_OPCODE 0xCB
#define EXTENSION_OFFSET 256

class CPU
{
public:
	CPU();

	void Step(uint8_t* memory)
	{
		//Fetch
		uint8_t encodedInstruction = memory[m_registers.PC++];

		//TODO handle instruction extension

		//Decode
		if (encodedInstruction == EXTENSION_OPCODE)
		{
			encodedInstruction = memory[m_registers.PC++] + EXTENSION_OFFSET;
		}

		const Instruction& instruction = m_instructions[encodedInstruction];

		//Execute
		instruction.m_func(instruction.m_mnemonic, &m_registers, memory);

		//TODO check for interrups
		//TODO adjust timings
	}

	void SetProgramCounter(unsigned short addr)
	{
		m_registers.PC = addr;
	}

	void Reset()
	{
		ClearRegisters();
	}

	// Tests want to set the register state directly.
#ifdef _DEBUG
	Registers& GetRegisters()
	{
		return m_registers;
	}
#endif // _DEBUG

private:
	typedef void (*InstructionFunc)(const char* mnemonic, Registers* registers, uint8_t* memory);

	struct Instruction
	{
		const char* m_mnemonic;
		const uint8_t m_length;
		const uint8_t m_duration;
		const InstructionFunc m_func;
	};

	void ClearRegisters()
	{
		m_registers.SP = DEFAULT_STACK_POINTER;
		//TODO do the other registers need to be initialized to the boot ROM values?
		m_registers.AF = 0;
		m_registers.BC = 0;
		m_registers.DE = 0;
		m_registers.HL = 0;
		m_registers.PC = 0;
	}

	Registers m_registers;

	const Instruction m_instructions[INSTRUCTION_SET_SIZE];
};

