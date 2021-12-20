#include "InstructionFunctions.h"
#include "Helpers.h"
#include <cassert>

namespace InstructionFunctions
{
	namespace Helpers
	{
		void SetHalfCarryFlag(bool subtract, const uint8_t& previousReg, const uint8_t& operand, Registers* registers);
		FORCE_INLINE uint16_t u16(uint8_t lsb, uint8_t msb)
		{
			return static_cast<uint16_t>(msb) << 8 | static_cast<uint16_t>(lsb);
		}

		FORCE_INLINE uint16_t Read16Bit(uint16_t& addr, uint8_t* memory)
		{
			uint8_t lsb = memory[addr++];
			uint8_t msb = memory[addr++];
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

		FORCE_INLINE void SetHalfCarryFlag(const uint8_t& previousReg, const uint8_t& operand, bool subtract, Registers* registers)
		{
			bool halfCarry = !subtract && ((static_cast<uint16_t>(previousReg) & 0xF) + (operand & 0xF) > 0xF) || subtract && static_cast<int16_t>(previousReg & 0xF) < static_cast<int16_t>(operand & 0xF);
			registers->SetFlag(Registers::Flags::h, halfCarry);
		}

		FORCE_INLINE void SetFlagsNoCarry(uint8_t previousReg, uint8_t operand, uint8_t result, bool subtract, Registers* registers)
		{
			SetZeroFlag(result, registers);

			SetHalfCarryFlag(previousReg, operand, subtract, registers);
			registers->SetFlag(Registers::Flags::n, subtract);
		}

		FORCE_INLINE void SetCarry(uint8_t previousReg, uint8_t operand, Registers* registers, bool subtract)
		{
			bool carry = (!subtract && (static_cast<uint16_t>(previousReg) + static_cast<uint16_t>(operand) > 0xFF)) || (subtract && (static_cast<uint16_t>(previousReg) < static_cast<uint16_t>(operand)));
			registers->SetFlag(Registers::Flags::cy, carry);
		}

		FORCE_INLINE void SetFlags(uint8_t previousReg, uint8_t operand, uint8_t result, bool subtract, Registers* registers)
		{
			SetCarry(previousReg, operand, registers, subtract);
			SetFlagsNoCarry(previousReg, operand, result, subtract, registers);
		}

		FORCE_INLINE void SetFlags16(uint16_t previousReg, uint16_t operand, Registers* registers)
		{
			uint32_t res = static_cast<uint32_t>(previousReg) + operand;
			registers->SetFlag(Registers::Flags::cy, res > 0xFFFF);

			bool halfCarry = (previousReg & 0xFFF) + (operand & 0xFFF) > 0xFFF;
			registers->SetFlag(Registers::Flags::h, halfCarry);

			registers->ResetFlag(Registers::Flags::n);
		}

		FORCE_INLINE void Addition(uint8_t& operand1, uint8_t operand2, Registers* registers)
		{
			uint8_t prevReg = operand1;
			operand1 += operand2;
			Helpers::SetFlags(prevReg, operand2, operand1, false, registers);
		}

		FORCE_INLINE void Subtraction(uint8_t& operand1, uint8_t operand2, Registers* registers)
		{
			uint8_t prevReg = operand1;
			operand1 -= operand2;
			Helpers::SetFlags(prevReg, operand2, operand1, true, registers);
		}

		FORCE_INLINE void CompareSubtraction(uint8_t operand1, uint8_t operand2, Registers* registers)
		{
			uint8_t copyOperand = operand1;
			Subtraction(copyOperand, operand2, registers);
		}

		FORCE_INLINE void SubtractionWithCarry(uint8_t& operand1, uint8_t operand2, Registers* registers)
		{
			uint8_t cy = registers->GetFlag(Registers::Flags::cy);
			operand2 += cy;
			Helpers::Subtraction(operand1, operand2, registers);
		}

		FORCE_INLINE void AdditionWithCarry(uint8_t& operand1, uint8_t operand2, Registers* registers)
		{
			uint8_t cy = registers->GetFlag(Registers::Flags::cy);
			Helpers::Addition(operand1, cy, registers);
			uint8_t tmpCy = registers->GetFlag(Registers::Flags::cy);
			uint8_t tmpH = registers->GetFlag(Registers::Flags::h);
			Helpers::Addition(operand1, operand2, registers);
			registers->OrFlag(Registers::Flags::cy, tmpCy);
			registers->OrFlag(Registers::Flags::h, tmpCy);
		}

		FORCE_INLINE void BitwiseAnd(uint8_t& operand1, uint8_t operand2, Registers* registers)
		{
			operand1 = operand1 & operand2;
			Helpers::SetZeroFlag(operand1, registers);
			registers->SetFlag(Registers::Flags::h);
			registers->ResetFlag(Registers::Flags::n);
			registers->ResetFlag(Registers::Flags::cy);
		}

		FORCE_INLINE void BitwiseXor(uint8_t& operand1, uint8_t operand2, Registers* registers)
		{
			operand1 = operand1 ^ operand2;
			Helpers::SetZeroFlag(operand1, registers);
			registers->ResetFlag(Registers::Flags::h);
			registers->ResetFlag(Registers::Flags::n);
			registers->ResetFlag(Registers::Flags::cy);
		}

		FORCE_INLINE void BitwiseOr(uint8_t& operand1, uint8_t operand2, Registers* registers)
		{
			operand1 = operand1 | operand2;
			Helpers::SetZeroFlag(operand1, registers);
			registers->ResetFlag(Registers::Flags::h);
			registers->ResetFlag(Registers::Flags::n);
			registers->ResetFlag(Registers::Flags::cy);
		}

		FORCE_INLINE void Call(uint16_t addr, Registers* registers, uint8_t* memory)
		{
			registers->SP -= 2;
			Helpers::Write16Bit(registers->PC, registers->SP, memory);
			registers->PC = addr;
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
	int8_t immediate = memory[registers->PC++];
	if (!registers->IsFlagSet(Registers::Flags::zf))
	{
		registers->PC += immediate;
	}
	LOG_INSTRUCTION(mnemonic, immediate);
}

void InstructionFunctions::JR_Z_n(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	int8_t immediate = memory[registers->PC++];
	if (registers->IsFlagSet(Registers::Flags::zf))
	{
		registers->PC += immediate;
	}
	LOG_INSTRUCTION(mnemonic, immediate);
}

void InstructionFunctions::JR_NC_n(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	int8_t immediate = memory[registers->PC++];
	if (!registers->IsFlagSet(Registers::Flags::cy))
	{
		registers->PC += immediate;
	}
	LOG_INSTRUCTION(mnemonic, immediate);
}

void InstructionFunctions::JR_C_n(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	int8_t immediate = memory[registers->PC++];
	if (registers->IsFlagSet(Registers::Flags::cy))
	{
		registers->PC += immediate;
	}
	LOG_INSTRUCTION(mnemonic, immediate);
}

void InstructionFunctions::JR_n(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	int8_t immediate = memory[registers->PC++];
	registers->PC += immediate;
	LOG_INSTRUCTION(mnemonic, immediate);
}

void InstructionFunctions::JP_NZ_nn(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint16_t immediate = Helpers::Read16Bit(registers->PC, memory);
	if (!registers->IsFlagSet(Registers::Flags::zf))
	{
		registers->PC = immediate;
	}
	LOG_INSTRUCTION(mnemonic, immediate);
}

void InstructionFunctions::JP_Z_nn(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint16_t immediate = Helpers::Read16Bit(registers->PC, memory);
	if (registers->IsFlagSet(Registers::Flags::zf))
	{
		registers->PC = immediate;
	}
	LOG_INSTRUCTION(mnemonic, immediate);
}

void InstructionFunctions::JP_NC_nn(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint16_t immediate = Helpers::Read16Bit(registers->PC, memory);
	if (!registers->IsFlagSet(Registers::Flags::cy))
	{
		registers->PC = immediate;
	}
	LOG_INSTRUCTION(mnemonic, immediate);
}

void InstructionFunctions::JP_C_nn(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint16_t immediate = Helpers::Read16Bit(registers->PC, memory);
	if (registers->IsFlagSet(Registers::Flags::cy))
	{
		registers->PC = immediate;
	}
	LOG_INSTRUCTION(mnemonic, immediate);
}

void InstructionFunctions::JP_nn(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint16_t immediate = Helpers::Read16Bit(registers->PC, memory);
	registers->PC = immediate;
	LOG_INSTRUCTION(mnemonic, immediate);
}

void InstructionFunctions::JP_HL(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->PC = registers->HL;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::CALL_NZ_nn(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint16_t immediate = Helpers::Read16Bit(registers->PC, memory);
	if (!registers->IsFlagSet(Registers::Flags::zf))
	{
		registers->SP -= 2;
		Helpers::Write16Bit(registers->PC, registers->SP, memory);
		registers->PC = immediate;
	}
	LOG_INSTRUCTION(mnemonic, immediate);
}

void InstructionFunctions::CALL_Z_nn(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint16_t immediate = Helpers::Read16Bit(registers->PC, memory);
	if (registers->IsFlagSet(Registers::Flags::zf))
	{
		Helpers::Call(immediate, registers, memory);
	}
	LOG_INSTRUCTION(mnemonic, immediate);
}

void InstructionFunctions::CALL_NC_nn(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint16_t immediate = Helpers::Read16Bit(registers->PC, memory);
	if (!registers->IsFlagSet(Registers::Flags::cy))
	{
		Helpers::Call(immediate, registers, memory);
	}
	LOG_INSTRUCTION(mnemonic, immediate);
}

void InstructionFunctions::CALL_C_nn(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint16_t immediate = Helpers::Read16Bit(registers->PC, memory);
	if (registers->IsFlagSet(Registers::Flags::cy))
	{
		Helpers::Call(immediate, registers, memory);
	}
	LOG_INSTRUCTION(mnemonic, immediate);
}

void InstructionFunctions::CALL_nn(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint16_t immediate = Helpers::Read16Bit(registers->PC, memory);
	Helpers::Call(immediate, registers, memory);
	LOG_INSTRUCTION(mnemonic, immediate);
}

void InstructionFunctions::RST_00(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::Call(0x00, registers, memory);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::RST_10(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::Call(0x10, registers, memory);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::RST_20(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::Call(0x20, registers, memory);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::RST_30(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::Call(0x30, registers, memory);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::RST_08(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::Call(0x08, registers, memory);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::RST_18(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::Call(0x18, registers, memory);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::RST_28(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::Call(0x28, registers, memory);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::RST_38(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::Call(0x38, registers, memory);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::RET_NZ(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	if (!registers->IsFlagSet(Registers::Flags::zf))
	{
		registers->PC = Helpers::Read16Bit(registers->SP, memory);
	}
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::RET_Z(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	if (registers->IsFlagSet(Registers::Flags::zf))
	{
		registers->PC = Helpers::Read16Bit(registers->SP, memory);
	}
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::RET_NC(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	if (!registers->IsFlagSet(Registers::Flags::cy))
	{
		registers->PC = Helpers::Read16Bit(registers->SP, memory);
	}
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::RET_C(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	if (registers->IsFlagSet(Registers::Flags::cy))
	{
		registers->PC = Helpers::Read16Bit(registers->SP, memory);
	}
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::RET(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->PC = Helpers::Read16Bit(registers->SP, memory);
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

void InstructionFunctions::LD_C_B(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->C = registers->B;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_C_C(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->C = registers->C;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_C_D(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->C = registers->D;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_C_E(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->C = registers->E;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_C_H(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->C = registers->H;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_C_L(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->C = registers->L;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_C_mHL(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->C = memory[registers->HL];
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_C_A(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->C = registers->A;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_D_B(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->D = registers->B;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_D_C(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->D = registers->C;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_D_D(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->D = registers->D;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_D_E(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->D = registers->E;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_D_H(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->D = registers->H;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_D_L(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->D = registers->L;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_D_mHL(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->D = memory[registers->HL];
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_D_A(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->D = registers->A;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_E_B(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->E = registers->B;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_E_C(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->E = registers->C;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_E_D(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->E = registers->D;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_E_E(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->E = registers->E;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_E_H(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->E = registers->H;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_E_L(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->E = registers->L;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_E_mHL(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->E = memory[registers->HL];
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_E_A(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->E = registers->A;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_H_B(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->H = registers->B;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_H_C(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->H = registers->C;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_H_D(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->H = registers->D;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_H_E(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->H = registers->E;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_H_H(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->H = registers->H;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_H_L(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->H = registers->L;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_H_mHL(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->H = memory[registers->HL];
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_H_A(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->H = registers->A;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_L_B(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->L = registers->B;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_L_C(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->L = registers->C;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_L_D(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->L = registers->D;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_L_E(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->L = registers->E;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_L_H(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->L = registers->H;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_L_L(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->L = registers->L;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_L_mHL(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->L = memory[registers->HL];
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_L_A(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->L = registers->A;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_mHL_B(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	memory[registers->HL] = registers->B;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_mHL_C(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	memory[registers->HL] = registers->C;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_mHL_D(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	memory[registers->HL] = registers->D;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_mHL_E(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	memory[registers->HL] = registers->E;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_mHL_H(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	memory[registers->HL] = registers->H;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_mHL_L(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	memory[registers->HL] = registers->L;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_mHL_A(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	memory[registers->HL] = registers->A;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_A_B(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->A = registers->B;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_A_C(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->A = registers->C;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_A_D(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->A = registers->D;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_A_E(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->A = registers->E;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_A_H(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->A = registers->H;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_A_L(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->A = registers->L;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_A_mHL(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->A = memory[registers->HL];
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::LD_A_A(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->A = registers->A;
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

void InstructionFunctions::LDH_mn_A(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint8_t immediate = memory[registers->PC++];
	uint16_t addr = Helpers::u16(immediate, 0xFF);
	memory[addr] = registers->A;
	LOG_INSTRUCTION(mnemonic, immediate);
}

void InstructionFunctions::LDH_A_mn(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint8_t immediate = memory[registers->PC++];
	uint16_t addr = Helpers::u16(immediate, 0xFF);
	registers->A = memory[addr];
	LOG_INSTRUCTION(mnemonic, immediate);
}

void InstructionFunctions::LDH_mC_A(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint16_t addr = Helpers::u16(registers->C, 0xFF);
	memory[addr] = registers->A;
	LOG_INSTRUCTION(mnemonic, immediate);
}

void InstructionFunctions::LDH_A_mC(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint16_t addr = Helpers::u16(registers->C, 0xFF);
	registers->A = memory[addr];
	LOG_INSTRUCTION(mnemonic, immediate);
}

void InstructionFunctions::LD_mnn_A(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint16_t immediate = Helpers::Read16Bit(registers->PC, memory);
	memory[immediate] = registers->A;
	LOG_INSTRUCTION(mnemonic, immediate);
}

void InstructionFunctions::LD_A_mnn(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint16_t immediate = Helpers::Read16Bit(registers->PC, memory);
	registers->A = memory[immediate];
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

void InstructionFunctions::ADD_A_B(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::Addition(registers->A, registers->B, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::ADD_A_C(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::Addition(registers->A, registers->C, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::ADD_A_D(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::Addition(registers->A, registers->D, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::ADD_A_E(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::Addition(registers->A, registers->E, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::ADD_A_H(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::Addition(registers->A, registers->H, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::ADD_A_L(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::Addition(registers->A, registers->L, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::ADD_A_mHL(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::Addition(registers->A, memory[registers->HL], registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::ADD_A_A(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::Addition(registers->A, registers->A, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::ADC_A_B(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::AdditionWithCarry(registers->A, registers->B, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::ADC_A_C(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::AdditionWithCarry(registers->A, registers->C, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::ADC_A_D(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::AdditionWithCarry(registers->A, registers->D, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::ADC_A_E(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::AdditionWithCarry(registers->A, registers->E, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::ADC_A_H(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::AdditionWithCarry(registers->A, registers->H, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::ADC_A_L(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::AdditionWithCarry(registers->A, registers->L, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::ADC_A_mHL(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::AdditionWithCarry(registers->A, memory[registers->HL], registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::ADC_A_A(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::AdditionWithCarry(registers->A, registers->A, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::SUB_A_B(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::Subtraction(registers->A, registers->B, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::SUB_A_C(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::Subtraction(registers->A, registers->C, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::SUB_A_D(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::Subtraction(registers->A, registers->D, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::SUB_A_E(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::Subtraction(registers->A, registers->E, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::SUB_A_H(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::Subtraction(registers->A, registers->H, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::SUB_A_L(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::Subtraction(registers->A, registers->L, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::SUB_A_mHL(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::Subtraction(registers->A, memory[registers->HL], registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::SUB_A_A(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::Subtraction(registers->A, registers->A, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::SBC_A_B(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::SubtractionWithCarry(registers->A, registers->B, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::SBC_A_C(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::SubtractionWithCarry(registers->A, registers->C, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::SBC_A_D(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::SubtractionWithCarry(registers->A, registers->D, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::SBC_A_E(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::SubtractionWithCarry(registers->A, registers->E, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::SBC_A_H(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::SubtractionWithCarry(registers->A, registers->H, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::SBC_A_L(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::SubtractionWithCarry(registers->A, registers->L, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::SBC_A_mHL(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::SubtractionWithCarry(registers->A, memory[registers->HL], registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::SBC_A_A(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::SubtractionWithCarry(registers->A, registers->A, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::AND_A_B(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::BitwiseAnd(registers->A, registers->B, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::AND_A_C(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::BitwiseAnd(registers->A, registers->C, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::AND_A_D(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::BitwiseAnd(registers->A, registers->D, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::AND_A_E(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::BitwiseAnd(registers->A, registers->E, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::AND_A_H(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::BitwiseAnd(registers->A, registers->H, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::AND_A_L(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::BitwiseAnd(registers->A, registers->L, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::AND_A_mHL(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::BitwiseAnd(registers->A, memory[registers->HL], registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::AND_A_A(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::BitwiseAnd(registers->A, registers->A, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::XOR_A_B(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::BitwiseXor(registers->A, registers->B, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::XOR_A_C(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::BitwiseXor(registers->A, registers->C, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::XOR_A_D(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::BitwiseXor(registers->A, registers->D, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::XOR_A_E(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::BitwiseXor(registers->A, registers->E, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::XOR_A_H(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::BitwiseXor(registers->A, registers->H, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::XOR_A_L(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::BitwiseXor(registers->A, registers->L, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::XOR_A_mHL(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::BitwiseXor(registers->A, memory[registers->HL], registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::XOR_A_A(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::BitwiseXor(registers->A, registers->A, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::OR_A_B(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::BitwiseOr(registers->A, registers->B, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::OR_A_C(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::BitwiseOr(registers->A, registers->C, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::OR_A_D(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::BitwiseOr(registers->A, registers->D, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::OR_A_E(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::BitwiseOr(registers->A, registers->E, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::OR_A_H(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::BitwiseOr(registers->A, registers->H, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::OR_A_L(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::BitwiseOr(registers->A, registers->L, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::OR_A_mHL(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::BitwiseOr(registers->A, memory[registers->HL], registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::OR_A_A(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::BitwiseOr(registers->A, registers->A, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::CP_A_B(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::CompareSubtraction(registers->A, registers->B, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::CP_A_C(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::CompareSubtraction(registers->A, registers->C, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::CP_A_D(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::CompareSubtraction(registers->A, registers->D, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::CP_A_E(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::CompareSubtraction(registers->A, registers->E, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::CP_A_H(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::CompareSubtraction(registers->A, registers->H, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::CP_A_L(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::CompareSubtraction(registers->A, registers->L, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::CP_A_mHL(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::CompareSubtraction(registers->A, memory[registers->HL], registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::CP_A_A(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	Helpers::CompareSubtraction(registers->A, registers->A, registers);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::ADD_A_n(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint8_t immediate = memory[registers->PC++];
	Helpers::Addition(registers->A, immediate, registers);
}

void InstructionFunctions::SUB_A_n(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint8_t immediate = memory[registers->PC++];
	Helpers::Subtraction(registers->A, immediate, registers);
}

void InstructionFunctions::AND_A_n(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint8_t immediate = memory[registers->PC++];
	Helpers::BitwiseAnd(registers->A, immediate, registers);
}

void InstructionFunctions::OR_A_n(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint8_t immediate = memory[registers->PC++];
	Helpers::BitwiseOr(registers->A, immediate, registers);
}

void InstructionFunctions::ADC_A_n(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint8_t immediate = memory[registers->PC++];
	Helpers::AdditionWithCarry(registers->A, immediate, registers);
}

void InstructionFunctions::SBC_A_n(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint8_t immediate = memory[registers->PC++];
	Helpers::SubtractionWithCarry(registers->A, immediate, registers);
}

void InstructionFunctions::XOR_A_n(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint8_t immediate = memory[registers->PC++];
	Helpers::BitwiseXor(registers->A, immediate, registers);
}

void InstructionFunctions::CP_A_n(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint8_t immediate = memory[registers->PC++];
	Helpers::CompareSubtraction(registers->A, immediate, registers);
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
	uint16_t immediate = Helpers::Read16Bit(registers->PC, memory);
	registers->BC = immediate;
	LOG_INSTRUCTION(mnemonic, immediate);
}

void InstructionFunctions::LD_DE_nn(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint16_t immediate = Helpers::Read16Bit(registers->PC, memory);
	registers->DE = immediate;
	LOG_INSTRUCTION(mnemonic, immediate);
}

void InstructionFunctions::LD_HL_nn(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint16_t immediate = Helpers::Read16Bit(registers->PC, memory);
	registers->HL = immediate;
	LOG_INSTRUCTION(mnemonic, immediate);
}

void InstructionFunctions::LD_SP_nn(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint16_t immediate = Helpers::Read16Bit(registers->PC, memory);
	registers->SP = immediate;
	LOG_INSTRUCTION(mnemonic, immediate);
}

void InstructionFunctions::LD_mnn_SP(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	uint16_t immediate = Helpers::Read16Bit(registers->PC, memory);
	Helpers::Write16Bit(registers->SP, immediate, memory);
	LOG_INSTRUCTION(mnemonic, immediate);
}

void InstructionFunctions::LD_HL_SP_n(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	int8_t immediate = memory[registers->PC++];
	bool subtraction = immediate < 0;
	uint8_t previous = static_cast<uint8_t>(registers->SP);
	registers->HL = registers->SP + immediate;
	registers->ResetFlag(Registers::Flags::zf);
	registers->ResetFlag(Registers::Flags::n);
	uint8_t operand = static_cast<uint8_t>(abs(immediate));
	Helpers::SetCarry(previous, operand, registers, subtraction);
	Helpers::SetHalfCarryFlag(previous, operand, subtraction, registers);
	LOG_INSTRUCTION(mnemonic, immediate);
}

void InstructionFunctions::LD_SP_HL(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->SP = registers->HL;
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::POP_BC(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->BC = Helpers::Read16Bit(registers->SP, memory);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::POP_DE(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->DE = Helpers::Read16Bit(registers->SP, memory);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::POP_HL(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->HL = Helpers::Read16Bit(registers->SP, memory);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::POP_AF(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->AF = Helpers::Read16Bit(registers->SP, memory);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::PUSH_BC(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->SP -= 2;
	Helpers::Write16Bit(registers->BC, registers->SP, memory);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::PUSH_DE(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->SP -= 2;
	Helpers::Write16Bit(registers->DE, registers->SP, memory);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::PUSH_HL(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->SP -= 2;
	Helpers::Write16Bit(registers->HL, registers->SP, memory);
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::PUSH_AF(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	registers->SP -= 2;
	Helpers::Write16Bit(registers->AF, registers->SP, memory);
	LOG_INSTRUCTION(mnemonic);
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

void InstructionFunctions::ADD_SP_n(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	int8_t immediate = memory[registers->PC++];
	bool subtraction = immediate < 0;
	uint8_t previous = static_cast<uint8_t>(registers->SP);
	registers->SP += immediate;
	registers->ResetFlag(Registers::Flags::n);
	uint8_t operand = static_cast<uint8_t>(abs(immediate));
	Helpers::SetCarry(previous, operand, registers, subtraction);
	Helpers::SetHalfCarryFlag(previous, operand, subtraction, registers);
	LOG_INSTRUCTION(mnemonic, immediate);
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
