#include "InstructionFunctions.h"
#include "Helpers.h"

namespace InstructionFunctions
{
	namespace Helpers
	{
		FORCE_INLINE uint16_t u16(uint8_t lsb, uint8_t msb)
		{
			return static_cast<uint16_t>(msb) << 8 | static_cast<uint16_t>(lsb);
		}

		FORCE_INLINE uint16_t DecodeImmediate16(uint16_t& pc, uint8_t* memory)
		{
			uint8_t lsb = memory[pc++];
			uint8_t msb = memory[pc++];
			return u16(lsb, msb);
		}

		FORCE_INLINE void SetZeroFlag(uint8_t result, Registers* registers)
		{
			if (result == 0)
			{
				registers->SetFlag(Registers::Flags::zf);
			}
			else
			{
				registers->ResetFlag(Registers::Flags::zf);
			}
		}

		//TODO check if faster without branches
		FORCE_INLINE void SetFlagsNoCarry(uint8_t previousReg, uint8_t operand, uint8_t result, bool subtract, Registers* registers)
		{
			SetZeroFlag(result, registers);

			bool halfCarry = !subtract && ((static_cast<uint16_t>(previousReg) & 0xF) + operand > 0xF) || subtract && (previousReg & 0xF) < operand;
			registers->SetFlag(Registers::Flags::h, halfCarry);
			registers->SetFlag(Registers::Flags::n, subtract);
		}
	}
}



void InstructionFunctions::LoadToRegister(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::NOP(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	LOG_INSTRUCTION(mnemonic);
}


//8 bit loads 
//-------------------------------------------------------------------------------------------------
void InstructionFunctions::LD_mBC_A(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	memory[registers->BC] = registers->A;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_mDE_A(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	memory[registers->DE] = registers->A;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_mHLinc_A(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	memory[registers->HL++] = registers->A;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_mHLdec_A(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	memory[registers->HL--] = registers->A;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_B_n(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint8_t immediate = memory[registers->PC++];
	registers->B = immediate;
	LOG_INSTRUCTION(mnemonic, immediate);
}

void InstructionFunctions::LD_D_n(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint8_t immediate = memory[registers->PC++];
	registers->D = immediate;
	LOG_INSTRUCTION(mnemonic, immediate);
}

void InstructionFunctions::LD_H_n(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint8_t immediate = memory[registers->PC++];
	registers->H = immediate;
	LOG_INSTRUCTION(mnemonic, immediate);
}

void InstructionFunctions::LD_mHL_n(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint8_t immediate = memory[registers->PC++];
	memory[registers->HL] = immediate;
	LOG_INSTRUCTION(mnemonic, immediate);
}

//8 bit arithmatic/logic 
//-------------------------------------------------------------------------------------------------
void InstructionFunctions::INC_B(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint8_t& reg = registers->B;
	uint8_t prevReg = reg;
	reg++;
	Helpers::SetFlagsNoCarry(prevReg, 1, reg, false, registers);

	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::INC_D(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint8_t& reg = registers->D;
	uint8_t prevReg = reg;
	reg++;
	Helpers::SetFlagsNoCarry(prevReg, 1, reg, false, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::INC_H(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint8_t& reg = registers->H;
	uint8_t prevReg = reg;
	reg++;
	Helpers::SetFlagsNoCarry(prevReg, 1, reg, false, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::INC_mHL(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint8_t& reg = memory[registers->HL];
	uint8_t prevReg = reg;
	reg++;
	Helpers::SetFlagsNoCarry(prevReg, 1, reg, false, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::DEC_B(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint8_t& reg = registers->B;
	uint8_t prevReg = reg;
	reg--;
	Helpers::SetFlagsNoCarry(prevReg, 1, reg, true, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::DEC_D(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint8_t& reg = registers->D;
	uint8_t prevReg = reg;
	reg--;
	Helpers::SetFlagsNoCarry(prevReg, 1, reg, true, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::DEC_H(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint8_t& reg = registers->H;
	uint8_t prevReg = reg;
	reg--;
	Helpers::SetFlagsNoCarry(prevReg, 1, reg, true, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::DEC_mHL(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint8_t& reg = memory[registers->HL];
	uint8_t prevReg = reg;
	reg--;
	Helpers::SetFlagsNoCarry(prevReg, 1, reg, true, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::RLCA(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint16_t reg = static_cast<uint16_t>(registers->A);
	reg = reg << 1;
	registers->A = static_cast<uint8_t>(reg);

	registers->SetFlag(Registers::Flags::cy, (reg & 0xFF00) != 0);
	registers->ResetFlag(Registers::Flags::n);
	registers->ResetFlag(Registers::Flags::h);
	Helpers::SetZeroFlag(registers->A, registers);

	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::RLA(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint8_t carry = registers->GetFlag(Registers::Flags::cy);
	uint16_t reg = static_cast<uint16_t>(registers->A);
	reg = reg << 1;
	reg = (reg & 0xFFFE) | carry;
	registers->A = static_cast<uint8_t>(reg);

	registers->SetFlag(Registers::Flags::cy, (reg & 0xFF00) != 0);
	registers->ResetFlag(Registers::Flags::n);
	registers->ResetFlag(Registers::Flags::h);
	Helpers::SetZeroFlag(registers->A, registers);

	LOG_INSTRUCTION(mnemonic);
}

//16 bit loads 
//-------------------------------------------------------------------------------------------------

void InstructionFunctions::LD_BC_nn(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint16_t immediate = Helpers::DecodeImmediate16(registers->PC, memory);
	registers->BC = immediate;
	LOG_INSTRUCTION(mnemonic, immediate);
}

void InstructionFunctions::LD_DE_nn(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint16_t immediate = Helpers::DecodeImmediate16(registers->PC, memory);
	registers->DE = immediate;
	LOG_INSTRUCTION(mnemonic, immediate);
}

void InstructionFunctions::LD_HL_nn(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint16_t immediate = Helpers::DecodeImmediate16(registers->PC, memory);
	registers->HL = immediate;
	LOG_INSTRUCTION(mnemonic, immediate);
}

void InstructionFunctions::LD_SP_nn(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint16_t immediate = Helpers::DecodeImmediate16(registers->PC, memory);
	registers->SP = immediate;
	LOG_INSTRUCTION(mnemonic, immediate);
}

//16 bit arithmatic/logic 
//-------------------------------------------------------------------------------------------------
void InstructionFunctions::INC_BC(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->BC++;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::INC_DE(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->DE++;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::INC_HL(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->HL++;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::INC_SP(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->SP++;
	LOG_INSTRUCTION(mnemonic);
}
