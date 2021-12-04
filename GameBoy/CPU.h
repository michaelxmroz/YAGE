#pragma once
#include "Registers.h"
#include "InstructionFunctions.h"

#define DEFAULT_STACK_POINTER 0xFFFE
#define INSTRUCTION_SET_SIZE 512

class CPU
{
public:
	CPU();

	void Step(uint8_t* memory)
	{
		//Fetch
		uint8_t encodedInstruction = memory[_registers.PC];

		_registers.PC++;

		//TODO handle instruction extension

		//Decode
		const Instruction& instruction = _instructions[encodedInstruction];

		//Execute
		instruction._func(instruction._mnemonic, instruction._length, &_registers, memory);

		//TODO check for interrups
		//TODO adjust timings
	}

	void SetProgramCounter(unsigned short addr)
	{
		_registers.PC = addr;
	}

	void Reset()
	{
		ClearRegisters();
	}

private:
	typedef void (*InstructionFunc)(const char* mnemonic, uint8_t length, Registers* registers, uint8_t* memory);

	struct Instruction
	{
		const char* _mnemonic;
		const uint8_t _length;
		const uint8_t _duration;
		const InstructionFunc _func;
	};

	void ClearRegisters()
	{
		_registers.SP = DEFAULT_STACK_POINTER;
		//TODO do the other registers need to be initialized to the boot ROM values?
		_registers.AF = 0;
		_registers.BC = 0;
		_registers.DE = 0;
		_registers.HL = 0;
		_registers.PC = 0;
	}

	Registers _registers;

	const Instruction _instructions[INSTRUCTION_SET_SIZE];
};

