#include "InstructionFunctions.h"
#include "Helpers.h"
#include <cassert>

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

		FORCE_INLINE void Write16Bit(uint16_t data, uint16_t addr, uint8_t* memory)
		{
			uint8_t lsb = static_cast<uint8_t>(data);
			uint8_t msb = static_cast<uint8_t>(data >> 8);
			memory[addr++] = lsb;
			memory[addr] = msb;
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

		FORCE_INLINE void SetFlagsNoCarry(uint8_t previousReg, uint8_t operand, uint8_t result, bool subtract, Registers* registers)
		{
			SetZeroFlag(result, registers);

			bool halfCarry = !subtract && ((static_cast<uint16_t>(previousReg) & 0xF) + (operand & 0xF) > 0xF) || subtract && (previousReg & 0xF) < operand;
			registers->SetFlag(Registers::Flags::h, halfCarry);
			registers->SetFlag(Registers::Flags::n, subtract);
		}

		FORCE_INLINE void SetFlags16(uint16_t previousReg, uint16_t operand, Registers* registers)
		{
			uint32_t res = static_cast<uint32_t>(previousReg) + operand;
			registers->SetFlag(Registers::Flags::cy, res > 0xFFFF);

			bool halfCarry = (previousReg & 0xFFF) + (operand & 0xFFF) > 0xFFF;
			registers->SetFlag(Registers::Flags::h, halfCarry);

			registers->ResetFlag(Registers::Flags::n);
		}
	}
}



void InstructionFunctions::UNIMPLEMENTED(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	LOG_INSTRUCTION(mnemonic);
	LOG_ERROR("Instruction not implemented");
}

void InstructionFunctions::NOP(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::SCF(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->ResetFlag(Registers::Flags::h);
	registers->ResetFlag(Registers::Flags::n);
	registers->SetFlag(Registers::Flags::cy);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::CCF(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->ResetFlag(Registers::Flags::h);
	registers->ResetFlag(Registers::Flags::n);
	registers->SetFlag(Registers::Flags::cy, !registers->IsFlagSet(Registers::Flags::cy));
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::JR_NZ_n(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint8_t immediate = memory[registers->PC++];
	if (!registers->IsFlagSet(Registers::Flags::zf))
	{
		registers->PC += immediate;
	}
	LOG_INSTRUCTION(mnemonic, immediate);
}

void InstructionFunctions::JR_Z_n(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint8_t immediate = memory[registers->PC++];
	if (registers->IsFlagSet(Registers::Flags::zf))
	{
		registers->PC += immediate;
	}
	LOG_INSTRUCTION(mnemonic, immediate);
}

void InstructionFunctions::JR_NC_n(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint8_t immediate = memory[registers->PC++];
	if (!registers->IsFlagSet(Registers::Flags::cy))
	{
		registers->PC += immediate;
	}
	LOG_INSTRUCTION(mnemonic, immediate);
}

void InstructionFunctions::JR_C_n(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint8_t immediate = memory[registers->PC++];
	if (registers->IsFlagSet(Registers::Flags::cy))
	{
		registers->PC += immediate;
	}
	LOG_INSTRUCTION(mnemonic, immediate);
}

void InstructionFunctions::JR_n(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint8_t immediate = memory[registers->PC++];
	registers->PC += immediate;
	LOG_INSTRUCTION(mnemonic, immediate);
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

void InstructionFunctions::LD_C_n(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint8_t immediate = memory[registers->PC++];
	registers->C = immediate;
	LOG_INSTRUCTION(mnemonic, immediate);
}

void InstructionFunctions::LD_E_n(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint8_t immediate = memory[registers->PC++];
	registers->E = immediate;
	LOG_INSTRUCTION(mnemonic, immediate);
}

void InstructionFunctions::LD_L_n(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint8_t immediate = memory[registers->PC++];
	registers->L = immediate;
	LOG_INSTRUCTION(mnemonic, immediate);
}

void InstructionFunctions::LD_A_n(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint8_t immediate = memory[registers->PC++];
	registers->A = immediate;
	LOG_INSTRUCTION(mnemonic, immediate);
}

void InstructionFunctions::LD_B_B(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->B = registers->B;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_B_C(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->B = registers->C;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_B_D(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->B = registers->D;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_B_E(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->B = registers->E;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_B_H(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->B = registers->H;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_B_L(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->B = registers->L;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_B_mHL(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->B = memory[registers->HL];
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_B_A(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->B = registers->A;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_A_mBC(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->A = memory[registers->BC];
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_A_mDE(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->A = memory[registers->DE];
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_A_mHLinc(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->A = memory[registers->HL++];
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_A_mHLdec(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->A = memory[registers->HL--];
	LOG_INSTRUCTION(mnemonic);
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

void InstructionFunctions::INC_C(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint8_t& reg = registers->C;
	uint8_t prevReg = reg;
	reg++;
	Helpers::SetFlagsNoCarry(prevReg, 1, reg, false, registers);

	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::INC_E(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint8_t& reg = registers->E;
	uint8_t prevReg = reg;
	reg++;
	Helpers::SetFlagsNoCarry(prevReg, 1, reg, false, registers);

	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::INC_L(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint8_t& reg = registers->L;
	uint8_t prevReg = reg;
	reg++;
	Helpers::SetFlagsNoCarry(prevReg, 1, reg, false, registers);

	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::INC_A(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint8_t& reg = registers->A;
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

void InstructionFunctions::DEC_C(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint8_t& reg = registers->C;
	uint8_t prevReg = reg;
	reg--;
	Helpers::SetFlagsNoCarry(prevReg, 1, reg, true, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::DEC_E(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint8_t& reg = registers->E;
	uint8_t prevReg = reg;
	reg--;
	Helpers::SetFlagsNoCarry(prevReg, 1, reg, true, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::DEC_L(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint8_t& reg = registers->L;
	uint8_t prevReg = reg;
	reg--;
	Helpers::SetFlagsNoCarry(prevReg, 1, reg, true, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::DEC_A(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint8_t& reg = registers->A;
	uint8_t prevReg = reg;
	reg--;
	Helpers::SetFlagsNoCarry(prevReg, 1, reg, true, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::CPL(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->A = ~registers->A;
	registers->SetFlag(Registers::Flags::n);
	registers->SetFlag(Registers::Flags::h);
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

void InstructionFunctions::RRCA(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->SetFlag(Registers::Flags::cy, registers->A & 0x1);
	registers->A = registers->A >> 1;

	registers->ResetFlag(Registers::Flags::n);
	registers->ResetFlag(Registers::Flags::h);
	Helpers::SetZeroFlag(registers->A, registers);

	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::RRA(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint8_t carry = registers->GetFlag(Registers::Flags::cy);

	registers->SetFlag(Registers::Flags::cy, registers->A & 0x1);
	registers->A = registers->A >> 1;
	carry = carry << 7;
	registers->A |= carry;

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

void InstructionFunctions::LD_mnn_SP(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint16_t immediate = Helpers::DecodeImmediate16(registers->PC, memory);
	Helpers::Write16Bit(registers->SP, immediate, memory);
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

void InstructionFunctions::ADD_HL_BC(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::SetFlags16(registers->HL, registers->BC, registers);
	registers->HL += registers->BC;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::ADD_HL_DE(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::SetFlags16(registers->HL, registers->DE, registers);
	registers->HL += registers->DE;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::ADD_HL_HL(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::SetFlags16(registers->HL, registers->HL, registers);
	registers->HL += registers->HL;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::ADD_HL_SP(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::SetFlags16(registers->HL, registers->SP, registers);
	registers->HL += registers->SP;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::DEC_BC(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->BC--;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::DEC_DE(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->DE--;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::DEC_HL(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->HL--;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::DEC_SP(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->SP--;
	LOG_INSTRUCTION(mnemonic);
}
