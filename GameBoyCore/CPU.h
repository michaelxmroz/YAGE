#pragma once
#include "Registers.h"
#include "InstructionFunctions.h"
#include "Interrupts.h"

#define DEFAULT_STACK_POINTER 0xFFFE
#define INSTRUCTION_SET_SIZE 512
#define EXTENSION_OPCODE 0xCB
#define EXTENSION_OFFSET 256
#define INTERRUPT_DURATION 5

#define EI_OPCODE 0xFB

class CPU
{
public:
	CPU(bool enableInterruptHandling = true);

	uint32_t Step(uint8_t* memory)
	{
		uint32_t mCycles = 0;
		bool handleInterrupts = true;

		if (m_registers.CpuState == Registers::State::Running)
		{		
			ExecuteInstruction(memory, mCycles, handleInterrupts);
		}

		if (m_InterruptHandlingEnabled)
		{
			bool hasInterrupt = Interrupts::ShouldHandleInterrupt(memory);

			//Wake up from low energy states
			if (hasInterrupt)
			{
				if (m_registers.CpuState == Registers::State::Halt || (m_registers.CpuState == Registers::State::Stop && Interrupts::ShouldHandleInterrupt(Interrupts::Types::Joypad, memory)))
				{
					m_registers.CpuState = Registers::State::Running;
					m_haltBug = true;
				}
			}

			//Handle interrupt
			if (handleInterrupts && hasInterrupt && m_registers.IMEF)
			{
				m_registers.IMEF = false;
				uint16_t jumpAddr = Interrupts::GetJumpAddrAndClear(memory);

				mCycles += INTERRUPT_DURATION;

				InstructionFunctions::Helpers::Call(jumpAddr, &m_registers, memory);
			}
		}

		return mCycles;
	}

	void ExecuteInstruction(uint8_t* memory, uint32_t& mCycles, bool& handleInterrupts)
	{
		//Fetch
		uint16_t encodedInstruction = memory[m_registers.PC++];

		//[Hardware] If the CPU was in a halt state and gets an interrupt request while interrupts are disabled in the IMEF register the PC does not increment properly
		if (m_haltBug)
		{
			m_registers.PC--;
			m_haltBug = false;
		}

		//Decode
		if (encodedInstruction == EXTENSION_OPCODE)
		{
			encodedInstruction = memory[m_registers.PC++] + EXTENSION_OFFSET;
		}

		const Instruction& instruction = m_instructions[encodedInstruction];

		mCycles += instruction.m_duration;

		//Execute
		mCycles += instruction.m_func(instruction.m_mnemonic, &m_registers, memory);

		//[Hardware] Interrupt handling is delayed by one cycle if EI was just called
		handleInterrupts = encodedInstruction != EI_OPCODE;
	}

	void SetProgramCounter(unsigned short addr)
	{
		m_registers.PC = addr;
	}

	void Reset()
	{
		m_haltBug = false;
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

	typedef uint32_t (*InstructionFunc)(const char* mnemonic, Registers* registers, uint8_t* memory);

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
		m_registers.IMEF = false;
		m_registers.CpuState = Registers::State::Running;
	}

	Registers m_registers;

	const Instruction m_instructions[INSTRUCTION_SET_SIZE];

	bool m_haltBug;
	const bool m_InterruptHandlingEnabled;
};

