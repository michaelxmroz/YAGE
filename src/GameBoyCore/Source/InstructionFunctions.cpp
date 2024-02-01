#include "InstructionFunctions.h"
#include "Helpers.h"
#include "Clock.h"
#include <cassert>

namespace InstructionFunctions
{
	namespace Helpers
	{
		FORCE_INLINE uint16_t u16(uint8_t lsb, uint8_t msb)
		{
			return static_cast<uint16_t>(msb) << 8 | static_cast<uint16_t>(lsb);
		}

		FORCE_INLINE uint16_t Read16Bit(uint16_t& addr, Memory& memory)
		{
			uint8_t lsb = memory[addr++];
			uint8_t msb = memory[addr++];
			return u16(lsb, msb);
		}

		FORCE_INLINE void Write16Bit(uint16_t data, uint16_t addr, Memory& memory)
		{
			uint8_t lsb = static_cast<uint8_t>(data);
			uint8_t msb = static_cast<uint8_t>(data >> 8);
			memory.Write(addr++, lsb);
			memory.Write(addr, msb);
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

		FORCE_INLINE void Call(uint16_t addr, Registers* registers, Memory& memory)
		{
			registers->SP -= 2;
			Helpers::Write16Bit(registers->PC, registers->SP, memory);
			registers->PC = addr;
		}

		FORCE_INLINE void RotateLeft(uint8_t& reg, Registers* registers)
		{
			uint16_t extendedReg = static_cast<uint16_t>(reg);
			extendedReg = extendedReg << 1;
			reg = static_cast<uint8_t>(extendedReg);
			bool carry = (extendedReg & 0xFF00) != 0;
			reg = reg | static_cast<uint8_t>(carry);
			registers->SetFlag(Registers::Flags::cy, carry);
			registers->ResetFlag(Registers::Flags::n);
			registers->ResetFlag(Registers::Flags::h);
			SetZeroFlag(reg, registers);
		}

		FORCE_INLINE void RotateLeftWithCarry(uint8_t& reg, Registers* registers)
		{
			uint8_t carry = registers->GetFlag(Registers::Flags::cy);
			uint16_t extendedReg = static_cast<uint16_t>(reg);
			extendedReg = extendedReg << 1;
			extendedReg = (extendedReg & 0xFFFE) | carry;
			reg = static_cast<uint8_t>(extendedReg);

			registers->SetFlag(Registers::Flags::cy, (extendedReg & 0xFF00) != 0);
			registers->ResetFlag(Registers::Flags::n);
			registers->ResetFlag(Registers::Flags::h);
			SetZeroFlag(reg, registers);
		}

		FORCE_INLINE void RotateRight(uint8_t& reg, Registers* registers)
		{
			uint8_t carry = reg & 0x1;
			registers->SetFlag(Registers::Flags::cy, carry);
			reg = reg >> 1;
			carry = carry << 7;
			reg |= carry;

			registers->ResetFlag(Registers::Flags::n);
			registers->ResetFlag(Registers::Flags::h);
			SetZeroFlag(reg, registers);
		}

		FORCE_INLINE void RotateRightWithCarry(uint8_t& reg, Registers* registers)
		{
			uint8_t carry = registers->GetFlag(Registers::Flags::cy);

			registers->SetFlag(Registers::Flags::cy, reg & 0x1);
			reg = reg >> 1;
			carry = carry << 7;
			reg |= carry;

			registers->ResetFlag(Registers::Flags::n);
			registers->ResetFlag(Registers::Flags::h);
			SetZeroFlag(reg, registers);
		}

		FORCE_INLINE void ShiftLeftArithmetic(uint8_t& reg, Registers* registers)
		{
			uint16_t extendedReg = static_cast<uint16_t>(reg);
			extendedReg = extendedReg << 1;
			reg = static_cast<uint8_t>(extendedReg);
			bool carry = (extendedReg & 0xFF00) != 0;
			registers->SetFlag(Registers::Flags::cy, carry);
			registers->ResetFlag(Registers::Flags::n);
			registers->ResetFlag(Registers::Flags::h);
			SetZeroFlag(reg, registers);
		}

		FORCE_INLINE void ShiftRightArithmetic(uint8_t& reg, Registers* registers)
		{
			uint8_t carry = reg & 0x1;
			registers->SetFlag(Registers::Flags::cy, carry);
			uint8_t sign = reg >> 7;
			reg = reg >> 1;

			reg |= (sign << 7);
			registers->ResetFlag(Registers::Flags::n);
			registers->ResetFlag(Registers::Flags::h);
			SetZeroFlag(reg, registers);
		}

		FORCE_INLINE void SwapNibbles(uint8_t& reg, Registers* registers)
		{
			uint8_t lhb = reg << 4;
			uint8_t rhb = reg >> 4;
			reg = lhb | rhb;

			registers->ResetFlag(Registers::Flags::cy);
			registers->ResetFlag(Registers::Flags::n);
			registers->ResetFlag(Registers::Flags::h);
			SetZeroFlag(reg, registers);
		}

		FORCE_INLINE void ShiftRightLogic(uint8_t& reg, Registers* registers)
		{
			uint8_t carry = reg & 0x1;
			registers->SetFlag(Registers::Flags::cy, carry);
			reg = reg >> 1;

			registers->ResetFlag(Registers::Flags::n);
			registers->ResetFlag(Registers::Flags::h);
			SetZeroFlag(reg, registers);
		}

		FORCE_INLINE void TestBit(uint8_t reg, uint8_t bit, Registers* registers)
		{
			uint8_t bitset = (reg >> bit) & 0x1;

			registers->ResetFlag(Registers::Flags::n);
			registers->SetFlag(Registers::Flags::h);
			registers->SetFlag(Registers::Flags::zf, bitset == 0);
		}

		FORCE_INLINE void ResetBit(uint8_t& reg, uint8_t bit, Registers* registers)
		{
			reg &= ~(0x1 << bit);
		}

		FORCE_INLINE void SetBit(uint8_t& reg, uint8_t bit, Registers* registers)
		{
			reg |= (0x1 << bit);
		}
	}
}



uint32_t InstructionFunctions::UNIMPLEMENTED(const char* mnemonic, Registers* registers, Memory& memory)
{
	LOG_INSTRUCTION(mnemonic);
	LOG_ERROR("Instruction not implemented");
    return 0;
}

uint32_t InstructionFunctions::NOP(const char* mnemonic, Registers* registers, Memory& memory)
{
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SCF(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->ResetFlag(Registers::Flags::h);
	registers->ResetFlag(Registers::Flags::n);
	registers->SetFlag(Registers::Flags::cy);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::CCF(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->ResetFlag(Registers::Flags::h);
	registers->ResetFlag(Registers::Flags::n);
	registers->SetFlag(Registers::Flags::cy, !registers->IsFlagSet(Registers::Flags::cy));
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::JR_NZ_n(const char* mnemonic, Registers* registers, Memory& memory)
{
	int8_t immediate = memory[registers->PC++];
	LOG_INSTRUCTION(mnemonic, immediate);
	if (!registers->IsFlagSet(Registers::Flags::zf))
	{
		registers->PC += immediate;
		return 1;
	}
    return 0;
}

uint32_t InstructionFunctions::JR_Z_n(const char* mnemonic, Registers* registers, Memory& memory)
{
	int8_t immediate = memory[registers->PC++];
	LOG_INSTRUCTION(mnemonic, immediate);
	if (registers->IsFlagSet(Registers::Flags::zf))
	{
		registers->PC += immediate;

	}
    return 0;
}

uint32_t InstructionFunctions::JR_NC_n(const char* mnemonic, Registers* registers, Memory& memory)
{
	int8_t immediate = memory[registers->PC++];
	LOG_INSTRUCTION(mnemonic, immediate);
	if (!registers->IsFlagSet(Registers::Flags::cy))
	{
		registers->PC += immediate;
		return 1;
	}
    return 0;
}

uint32_t InstructionFunctions::JR_C_n(const char* mnemonic, Registers* registers, Memory& memory)
{
	int8_t immediate = memory[registers->PC++];
	LOG_INSTRUCTION(mnemonic, immediate);
	if (registers->IsFlagSet(Registers::Flags::cy))
	{
		registers->PC += immediate;
		return 1;
	}
    return 0;
}

uint32_t InstructionFunctions::JR_n(const char* mnemonic, Registers* registers, Memory& memory)
{
	int8_t immediate = memory[registers->PC++];
	registers->PC += immediate;
	LOG_INSTRUCTION(mnemonic, immediate);
    return 0;
}

uint32_t InstructionFunctions::JP_NZ_nn(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint16_t immediate = Helpers::Read16Bit(registers->PC, memory);
	LOG_INSTRUCTION(mnemonic, immediate);
	if (!registers->IsFlagSet(Registers::Flags::zf))
	{
		registers->PC = immediate;
		return 1;
	}
    return 0;
}

uint32_t InstructionFunctions::JP_Z_nn(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint16_t immediate = Helpers::Read16Bit(registers->PC, memory);
	LOG_INSTRUCTION(mnemonic, immediate);
	if (registers->IsFlagSet(Registers::Flags::zf))
	{
		registers->PC = immediate;
		return 1;
	}
    return 0;
}

uint32_t InstructionFunctions::JP_NC_nn(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint16_t immediate = Helpers::Read16Bit(registers->PC, memory);
	LOG_INSTRUCTION(mnemonic, immediate);
	if (!registers->IsFlagSet(Registers::Flags::cy))
	{
		registers->PC = immediate;
		return 1;
	}
    return 0;
}

uint32_t InstructionFunctions::JP_C_nn(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint16_t immediate = Helpers::Read16Bit(registers->PC, memory);
	LOG_INSTRUCTION(mnemonic, immediate);
	if (registers->IsFlagSet(Registers::Flags::cy))
	{
		registers->PC = immediate;
		return 1;
	}
    return 0;
}

uint32_t InstructionFunctions::JP_nn(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint16_t immediate = Helpers::Read16Bit(registers->PC, memory);
	registers->PC = immediate;
	LOG_INSTRUCTION(mnemonic, immediate);
    return 0;
}

uint32_t InstructionFunctions::JP_HL(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->PC = registers->HL;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::CALL_NZ_nn(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint16_t immediate = Helpers::Read16Bit(registers->PC, memory);
	LOG_INSTRUCTION(mnemonic, immediate);
	if (!registers->IsFlagSet(Registers::Flags::zf))
	{
		Helpers::Call(immediate, registers, memory);
		return 3;
	}
    return 0;
}

uint32_t InstructionFunctions::CALL_Z_nn(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint16_t immediate = Helpers::Read16Bit(registers->PC, memory);
	LOG_INSTRUCTION(mnemonic, immediate);
	if (registers->IsFlagSet(Registers::Flags::zf))
	{
		Helpers::Call(immediate, registers, memory);
		return 3;
	}
    return 0;
}

uint32_t InstructionFunctions::CALL_NC_nn(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint16_t immediate = Helpers::Read16Bit(registers->PC, memory);
	LOG_INSTRUCTION(mnemonic, immediate);
	if (!registers->IsFlagSet(Registers::Flags::cy))
	{
		Helpers::Call(immediate, registers, memory);
		return 3;
	}
    return 0;
}

uint32_t InstructionFunctions::CALL_C_nn(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint16_t immediate = Helpers::Read16Bit(registers->PC, memory);
	LOG_INSTRUCTION(mnemonic, immediate);
	if (registers->IsFlagSet(Registers::Flags::cy))
	{
		Helpers::Call(immediate, registers, memory);
		return 3;
	}
    return 0;
}

uint32_t InstructionFunctions::CALL_nn(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint16_t immediate = Helpers::Read16Bit(registers->PC, memory);
	Helpers::Call(immediate, registers, memory);
	LOG_INSTRUCTION(mnemonic, immediate);
    return 0;
}

uint32_t InstructionFunctions::RST_00(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::Call(0x00, registers, memory);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RST_10(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::Call(0x10, registers, memory);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RST_20(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::Call(0x20, registers, memory);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RST_30(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::Call(0x30, registers, memory);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RST_08(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::Call(0x08, registers, memory);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RST_18(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::Call(0x18, registers, memory);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RST_28(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::Call(0x28, registers, memory);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RST_38(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::Call(0x38, registers, memory);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RET_NZ(const char* mnemonic, Registers* registers, Memory& memory)
{
	LOG_INSTRUCTION(mnemonic);
	if (!registers->IsFlagSet(Registers::Flags::zf))
	{
		registers->PC = Helpers::Read16Bit(registers->SP, memory);
		return 3;
	}
    return 0;
}

uint32_t InstructionFunctions::RET_Z(const char* mnemonic, Registers* registers, Memory& memory)
{
	LOG_INSTRUCTION(mnemonic);
	if (registers->IsFlagSet(Registers::Flags::zf))
	{
		registers->PC = Helpers::Read16Bit(registers->SP, memory);
		return 3;
	}
    return 0;
}

uint32_t InstructionFunctions::RET_NC(const char* mnemonic, Registers* registers, Memory& memory)
{
	LOG_INSTRUCTION(mnemonic);
	if (!registers->IsFlagSet(Registers::Flags::cy))
	{
		registers->PC = Helpers::Read16Bit(registers->SP, memory);
		return 3;
	}
    return 0;
}

uint32_t InstructionFunctions::RET_C(const char* mnemonic, Registers* registers, Memory& memory)
{
	LOG_INSTRUCTION(mnemonic);
	if (registers->IsFlagSet(Registers::Flags::cy))
	{
		registers->PC = Helpers::Read16Bit(registers->SP, memory);
		return 3;
	}
    return 0;
}

uint32_t InstructionFunctions::RET(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->PC = Helpers::Read16Bit(registers->SP, memory);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RETI(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->PC = Helpers::Read16Bit(registers->SP, memory);
	registers->IMEF = true;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::EI(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->IMEF = true;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::DI(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->IMEF = false;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::HALT(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->CpuState = Registers::State::Halt;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::STOP(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->PC++;
	registers->CpuState = Registers::State::Stop;
	Clock::ResetDivider(&memory, 0x00, 0, 0, nullptr);
	LOG_INSTRUCTION(mnemonic);
	return 0;
}


//8 bit loads 
//-------------------------------------------------------------------------------------------------
uint32_t InstructionFunctions::LD_mBC_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	memory.Write(registers->BC, registers->A);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_mDE_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	memory.Write(registers->DE, registers->A);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_mHLinc_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	memory.Write(registers->HL++, registers->A);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_mHLdec_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	memory.Write(registers->HL--, registers->A);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_B_n(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t immediate = memory[registers->PC++];
	registers->B = immediate;
	LOG_INSTRUCTION(mnemonic, immediate);
    return 0;
}

uint32_t InstructionFunctions::LD_D_n(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t immediate = memory[registers->PC++];
	registers->D = immediate;
	LOG_INSTRUCTION(mnemonic, immediate);
    return 0;
}

uint32_t InstructionFunctions::LD_H_n(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t immediate = memory[registers->PC++];
	registers->H = immediate;
	LOG_INSTRUCTION(mnemonic, immediate);
    return 0;
}

uint32_t InstructionFunctions::LD_mHL_n(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t immediate = memory[registers->PC++];
	memory.Write(registers->HL, immediate);
	LOG_INSTRUCTION(mnemonic, immediate);
    return 0;
}

uint32_t InstructionFunctions::LD_C_n(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t immediate = memory[registers->PC++];
	registers->C = immediate;
	LOG_INSTRUCTION(mnemonic, immediate);
    return 0;
}

uint32_t InstructionFunctions::LD_E_n(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t immediate = memory[registers->PC++];
	registers->E = immediate;
	LOG_INSTRUCTION(mnemonic, immediate);
    return 0;
}

uint32_t InstructionFunctions::LD_L_n(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t immediate = memory[registers->PC++];
	registers->L = immediate;
	LOG_INSTRUCTION(mnemonic, immediate);
    return 0;
}

uint32_t InstructionFunctions::LD_A_n(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t immediate = memory[registers->PC++];
	registers->A = immediate;
	LOG_INSTRUCTION(mnemonic, immediate);
    return 0;
}

uint32_t InstructionFunctions::LD_B_B(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->B = registers->B;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_B_C(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->B = registers->C;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_B_D(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->B = registers->D;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_B_E(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->B = registers->E;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_B_H(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->B = registers->H;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_B_L(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->B = registers->L;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_B_mHL(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->B = memory[registers->HL];
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_B_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->B = registers->A;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_C_B(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->C = registers->B;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_C_C(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->C = registers->C;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_C_D(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->C = registers->D;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_C_E(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->C = registers->E;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_C_H(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->C = registers->H;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_C_L(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->C = registers->L;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_C_mHL(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->C = memory[registers->HL];
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_C_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->C = registers->A;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_D_B(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->D = registers->B;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_D_C(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->D = registers->C;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_D_D(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->D = registers->D;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_D_E(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->D = registers->E;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_D_H(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->D = registers->H;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_D_L(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->D = registers->L;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_D_mHL(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->D = memory[registers->HL];
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_D_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->D = registers->A;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_E_B(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->E = registers->B;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_E_C(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->E = registers->C;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_E_D(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->E = registers->D;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_E_E(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->E = registers->E;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_E_H(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->E = registers->H;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_E_L(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->E = registers->L;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_E_mHL(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->E = memory[registers->HL];
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_E_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->E = registers->A;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_H_B(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->H = registers->B;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_H_C(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->H = registers->C;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_H_D(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->H = registers->D;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_H_E(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->H = registers->E;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_H_H(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->H = registers->H;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_H_L(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->H = registers->L;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_H_mHL(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->H = memory[registers->HL];
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_H_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->H = registers->A;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_L_B(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->L = registers->B;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_L_C(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->L = registers->C;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_L_D(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->L = registers->D;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_L_E(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->L = registers->E;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_L_H(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->L = registers->H;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_L_L(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->L = registers->L;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_L_mHL(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->L = memory[registers->HL];
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_L_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->L = registers->A;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_mHL_B(const char* mnemonic, Registers* registers, Memory& memory)
{
	memory.Write(registers->HL, registers->B);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_mHL_C(const char* mnemonic, Registers* registers, Memory& memory)
{
	memory.Write(registers->HL, registers->C);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_mHL_D(const char* mnemonic, Registers* registers, Memory& memory)
{
	memory.Write(registers->HL, registers->D);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_mHL_E(const char* mnemonic, Registers* registers, Memory& memory)
{
	memory.Write(registers->HL, registers->E);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_mHL_H(const char* mnemonic, Registers* registers, Memory& memory)
{
	memory.Write(registers->HL, registers->H);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_mHL_L(const char* mnemonic, Registers* registers, Memory& memory)
{
	memory.Write(registers->HL, registers->L);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_mHL_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	memory.Write(registers->HL, registers->A);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_A_B(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->A = registers->B;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_A_C(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->A = registers->C;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_A_D(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->A = registers->D;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_A_E(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->A = registers->E;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_A_H(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->A = registers->H;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_A_L(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->A = registers->L;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_A_mHL(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->A = memory[registers->HL];
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_A_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->A = registers->A;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_A_mBC(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->A = memory[registers->BC];
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_A_mDE(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->A = memory[registers->DE];
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_A_mHLinc(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->A = memory[registers->HL++];
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_A_mHLdec(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->A = memory[registers->HL--];
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LDH_mn_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t immediate = memory[registers->PC++];
	uint16_t addr = Helpers::u16(immediate, 0xFF);
	memory.Write(addr, registers->A);
	LOG_INSTRUCTION(mnemonic, immediate);
    return 0;
}

uint32_t InstructionFunctions::LDH_A_mn(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t immediate = memory[registers->PC++];
	uint16_t addr = Helpers::u16(immediate, 0xFF);
	registers->A = memory[addr];
	LOG_INSTRUCTION(mnemonic, immediate);
    return 0;
}

uint32_t InstructionFunctions::LDH_mC_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint16_t addr = Helpers::u16(registers->C, 0xFF);
	memory.Write(addr, registers->A);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LDH_A_mC(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint16_t addr = Helpers::u16(registers->C, 0xFF);
	registers->A = memory[addr];
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::LD_mnn_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint16_t immediate = Helpers::Read16Bit(registers->PC, memory);
	memory.Write(immediate, registers->A);
	LOG_INSTRUCTION(mnemonic, immediate);
    return 0;
}

uint32_t InstructionFunctions::LD_A_mnn(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint16_t immediate = Helpers::Read16Bit(registers->PC, memory);
	registers->A = memory[immediate];
	LOG_INSTRUCTION(mnemonic, immediate);
	return 0;
}

//8 bit arithmatic/logic 
//-------------------------------------------------------------------------------------------------
uint32_t InstructionFunctions::INC_B(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t& reg = registers->B;
	uint8_t prevReg = reg;
	reg++;
	Helpers::SetFlagsNoCarry(prevReg, 1, reg, false, registers);

	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::INC_D(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t& reg = registers->D;
	uint8_t prevReg = reg;
	reg++;
	Helpers::SetFlagsNoCarry(prevReg, 1, reg, false, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::INC_H(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t& reg = registers->H;
	uint8_t prevReg = reg;
	reg++;
	Helpers::SetFlagsNoCarry(prevReg, 1, reg, false, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::INC_mHL(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t prevReg = memory[registers->HL];
	uint8_t reg = prevReg + 1;
	memory.Write(registers->HL, reg);
	Helpers::SetFlagsNoCarry(prevReg, 1, reg, false, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::INC_C(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t& reg = registers->C;
	uint8_t prevReg = reg;
	reg++;
	Helpers::SetFlagsNoCarry(prevReg, 1, reg, false, registers);

	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::INC_E(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t& reg = registers->E;
	uint8_t prevReg = reg;
	reg++;
	Helpers::SetFlagsNoCarry(prevReg, 1, reg, false, registers);

	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::INC_L(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t& reg = registers->L;
	uint8_t prevReg = reg;
	reg++;
	Helpers::SetFlagsNoCarry(prevReg, 1, reg, false, registers);

	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::INC_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t& reg = registers->A;
	uint8_t prevReg = reg;
	reg++;
	Helpers::SetFlagsNoCarry(prevReg, 1, reg, false, registers);

	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::DEC_B(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t& reg = registers->B;
	uint8_t prevReg = reg;
	reg--;
	Helpers::SetFlagsNoCarry(prevReg, 1, reg, true, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::DEC_D(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t& reg = registers->D;
	uint8_t prevReg = reg;
	reg--;
	Helpers::SetFlagsNoCarry(prevReg, 1, reg, true, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::DEC_H(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t& reg = registers->H;
	uint8_t prevReg = reg;
	reg--;
	Helpers::SetFlagsNoCarry(prevReg, 1, reg, true, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::DEC_mHL(const char* mnemonic, Registers* registers, Memory& memory)
{	 
	uint8_t prevReg = memory[registers->HL];
	uint8_t reg = prevReg - 1;
	memory.Write(registers->HL, reg);
	Helpers::SetFlagsNoCarry(prevReg, 1, reg, true, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::DEC_C(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t& reg = registers->C;
	uint8_t prevReg = reg;
	reg--;
	Helpers::SetFlagsNoCarry(prevReg, 1, reg, true, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::DEC_E(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t& reg = registers->E;
	uint8_t prevReg = reg;
	reg--;
	Helpers::SetFlagsNoCarry(prevReg, 1, reg, true, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::DEC_L(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t& reg = registers->L;
	uint8_t prevReg = reg;
	reg--;
	Helpers::SetFlagsNoCarry(prevReg, 1, reg, true, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::DEC_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t& reg = registers->A;
	uint8_t prevReg = reg;
	reg--;
	Helpers::SetFlagsNoCarry(prevReg, 1, reg, true, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::CPL(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->A = ~registers->A;
	registers->SetFlag(Registers::Flags::n);
	registers->SetFlag(Registers::Flags::h);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::ADD_A_B(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::Addition(registers->A, registers->B, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::ADD_A_C(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::Addition(registers->A, registers->C, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::ADD_A_D(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::Addition(registers->A, registers->D, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::ADD_A_E(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::Addition(registers->A, registers->E, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::ADD_A_H(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::Addition(registers->A, registers->H, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::ADD_A_L(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::Addition(registers->A, registers->L, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::ADD_A_mHL(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::Addition(registers->A, memory[registers->HL], registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::ADD_A_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::Addition(registers->A, registers->A, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::ADC_A_B(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::AdditionWithCarry(registers->A, registers->B, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::ADC_A_C(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::AdditionWithCarry(registers->A, registers->C, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::ADC_A_D(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::AdditionWithCarry(registers->A, registers->D, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::ADC_A_E(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::AdditionWithCarry(registers->A, registers->E, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::ADC_A_H(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::AdditionWithCarry(registers->A, registers->H, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::ADC_A_L(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::AdditionWithCarry(registers->A, registers->L, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::ADC_A_mHL(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::AdditionWithCarry(registers->A, memory[registers->HL], registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::ADC_A_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::AdditionWithCarry(registers->A, registers->A, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SUB_A_B(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::Subtraction(registers->A, registers->B, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SUB_A_C(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::Subtraction(registers->A, registers->C, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SUB_A_D(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::Subtraction(registers->A, registers->D, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SUB_A_E(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::Subtraction(registers->A, registers->E, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SUB_A_H(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::Subtraction(registers->A, registers->H, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SUB_A_L(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::Subtraction(registers->A, registers->L, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SUB_A_mHL(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::Subtraction(registers->A, memory[registers->HL], registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SUB_A_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::Subtraction(registers->A, registers->A, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SBC_A_B(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SubtractionWithCarry(registers->A, registers->B, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SBC_A_C(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SubtractionWithCarry(registers->A, registers->C, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SBC_A_D(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SubtractionWithCarry(registers->A, registers->D, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SBC_A_E(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SubtractionWithCarry(registers->A, registers->E, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SBC_A_H(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SubtractionWithCarry(registers->A, registers->H, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SBC_A_L(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SubtractionWithCarry(registers->A, registers->L, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SBC_A_mHL(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SubtractionWithCarry(registers->A, memory[registers->HL], registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SBC_A_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SubtractionWithCarry(registers->A, registers->A, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::AND_A_B(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::BitwiseAnd(registers->A, registers->B, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::AND_A_C(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::BitwiseAnd(registers->A, registers->C, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::AND_A_D(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::BitwiseAnd(registers->A, registers->D, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::AND_A_E(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::BitwiseAnd(registers->A, registers->E, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::AND_A_H(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::BitwiseAnd(registers->A, registers->H, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::AND_A_L(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::BitwiseAnd(registers->A, registers->L, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::AND_A_mHL(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::BitwiseAnd(registers->A, memory[registers->HL], registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::AND_A_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::BitwiseAnd(registers->A, registers->A, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::XOR_A_B(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::BitwiseXor(registers->A, registers->B, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::XOR_A_C(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::BitwiseXor(registers->A, registers->C, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::XOR_A_D(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::BitwiseXor(registers->A, registers->D, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::XOR_A_E(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::BitwiseXor(registers->A, registers->E, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::XOR_A_H(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::BitwiseXor(registers->A, registers->H, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::XOR_A_L(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::BitwiseXor(registers->A, registers->L, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::XOR_A_mHL(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::BitwiseXor(registers->A, memory[registers->HL], registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::XOR_A_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::BitwiseXor(registers->A, registers->A, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::OR_A_B(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::BitwiseOr(registers->A, registers->B, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::OR_A_C(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::BitwiseOr(registers->A, registers->C, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::OR_A_D(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::BitwiseOr(registers->A, registers->D, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::OR_A_E(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::BitwiseOr(registers->A, registers->E, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::OR_A_H(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::BitwiseOr(registers->A, registers->H, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::OR_A_L(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::BitwiseOr(registers->A, registers->L, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::OR_A_mHL(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::BitwiseOr(registers->A, memory[registers->HL], registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::OR_A_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::BitwiseOr(registers->A, registers->A, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::CP_A_B(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::CompareSubtraction(registers->A, registers->B, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::CP_A_C(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::CompareSubtraction(registers->A, registers->C, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::CP_A_D(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::CompareSubtraction(registers->A, registers->D, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::CP_A_E(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::CompareSubtraction(registers->A, registers->E, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::CP_A_H(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::CompareSubtraction(registers->A, registers->H, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::CP_A_L(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::CompareSubtraction(registers->A, registers->L, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::CP_A_mHL(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::CompareSubtraction(registers->A, memory[registers->HL], registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::CP_A_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::CompareSubtraction(registers->A, registers->A, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::ADD_A_n(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t immediate = memory[registers->PC++];
	Helpers::Addition(registers->A, immediate, registers);
	LOG_INSTRUCTION(mnemonic, immediate);
    return 0;
}

uint32_t InstructionFunctions::SUB_A_n(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t immediate = memory[registers->PC++];
	Helpers::Subtraction(registers->A, immediate, registers);
	LOG_INSTRUCTION(mnemonic, immediate);
    return 0;
}

uint32_t InstructionFunctions::AND_A_n(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t immediate = memory[registers->PC++];
	Helpers::BitwiseAnd(registers->A, immediate, registers);
	LOG_INSTRUCTION(mnemonic, immediate);
    return 0;
}

uint32_t InstructionFunctions::OR_A_n(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t immediate = memory[registers->PC++];
	Helpers::BitwiseOr(registers->A, immediate, registers);
	LOG_INSTRUCTION(mnemonic, immediate);
    return 0;
}

uint32_t InstructionFunctions::ADC_A_n(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t immediate = memory[registers->PC++];
	Helpers::AdditionWithCarry(registers->A, immediate, registers);
	LOG_INSTRUCTION(mnemonic, immediate);
    return 0;
}

uint32_t InstructionFunctions::SBC_A_n(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t immediate = memory[registers->PC++];
	Helpers::SubtractionWithCarry(registers->A, immediate, registers);
	LOG_INSTRUCTION(mnemonic, immediate);
    return 0;
}

uint32_t InstructionFunctions::XOR_A_n(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t immediate = memory[registers->PC++];
	Helpers::BitwiseXor(registers->A, immediate, registers);
	LOG_INSTRUCTION(mnemonic, immediate);
    return 0;
}

uint32_t InstructionFunctions::CP_A_n(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t immediate = memory[registers->PC++];
	Helpers::CompareSubtraction(registers->A, immediate, registers);
	LOG_INSTRUCTION(mnemonic, immediate);
    return 0;
}

uint32_t InstructionFunctions::DAA(const char* mnemonic, Registers* registers, Memory& memory)
{
	bool subtraction = registers->GetFlag(Registers::Flags::n);
	bool halfCarry = registers->GetFlag(Registers::Flags::h);
	bool carry = registers->GetFlag(Registers::Flags::cy);
	uint8_t correction = 0;
	if (halfCarry || (!subtraction && (registers->A & 0xf) > 9)) {
		correction |= 0x6;
	}

	if (carry || (!subtraction && registers->A > 0x99)) {
		correction |= 0x60;
		registers->SetFlag(Registers::Flags::cy);
	}

	registers->A += subtraction ? -correction : correction;

	registers->A &= 0xff;

	Helpers::SetZeroFlag(registers->A, registers);
	registers->ResetFlag(Registers::Flags::h);
	
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RLCA(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::RotateLeft(registers->A, registers);
	registers->SetFlag(Registers::Flags::zf, 0);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RLA(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::RotateLeftWithCarry(registers->A, registers);
	registers->SetFlag(Registers::Flags::zf, 0);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RRCA(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::RotateRight(registers->A, registers);
	registers->SetFlag(Registers::Flags::zf, 0);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RRA(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::RotateRightWithCarry(registers->A, registers);
	registers->SetFlag(Registers::Flags::zf, 0);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RLC_B(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::RotateLeft(registers->B, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RLC_C(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::RotateLeft(registers->C, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RLC_D(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::RotateLeft(registers->D, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RLC_E(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::RotateLeft(registers->E, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RLC_H(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::RotateLeft(registers->H, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RLC_L(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::RotateLeft(registers->L, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RLC_mHL(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t val = memory[registers->HL];
	Helpers::RotateLeft(val, registers);
	memory.Write(registers->HL, val);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RLC_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::RotateLeft(registers->A, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RRC_B(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::RotateRight(registers->B, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RRC_C(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::RotateRight(registers->C, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RRC_D(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::RotateRight(registers->D, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RRC_E(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::RotateRight(registers->E, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RRC_H(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::RotateRight(registers->H, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RRC_L(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::RotateRight(registers->L, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RRC_mHL(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t val = memory[registers->HL];
	Helpers::RotateRight(val, registers);
	memory.Write(registers->HL, val);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RRC_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::RotateRight(registers->A, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RL_B(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::RotateLeftWithCarry(registers->B, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RL_C(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::RotateLeftWithCarry(registers->C, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RL_D(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::RotateLeftWithCarry(registers->D, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RL_E(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::RotateLeftWithCarry(registers->E, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RL_H(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::RotateLeftWithCarry(registers->H, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RL_L(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::RotateLeftWithCarry(registers->L, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RL_mHL(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t val = memory[registers->HL];
	Helpers::RotateLeftWithCarry(val, registers);
	memory.Write(registers->HL, val);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RL_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::RotateLeftWithCarry(registers->A, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RR_B(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::RotateRightWithCarry(registers->B, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RR_C(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::RotateRightWithCarry(registers->C, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RR_D(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::RotateRightWithCarry(registers->D, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RR_E(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::RotateRightWithCarry(registers->E, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RR_H(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::RotateRightWithCarry(registers->H, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RR_L(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::RotateRightWithCarry(registers->L, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RR_mHL(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t val = memory[registers->HL];
	Helpers::RotateRightWithCarry(val, registers);
	memory.Write(registers->HL, val);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RR_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::RotateRightWithCarry(registers->A, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SLA_B(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ShiftLeftArithmetic(registers->B, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SLA_C(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ShiftLeftArithmetic(registers->C, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SLA_D(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ShiftLeftArithmetic(registers->D, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SLA_E(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ShiftLeftArithmetic(registers->E, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SLA_H(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ShiftLeftArithmetic(registers->H, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SLA_L(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ShiftLeftArithmetic(registers->L, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SLA_mHL(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t val = memory[registers->HL];
	Helpers::ShiftLeftArithmetic(val, registers);
	memory.Write(registers->HL, val);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SLA_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ShiftLeftArithmetic(registers->A, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SRA_B(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ShiftRightArithmetic(registers->B, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SRA_C(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ShiftRightArithmetic(registers->C, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SRA_D(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ShiftRightArithmetic(registers->D, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SRA_E(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ShiftRightArithmetic(registers->E, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SRA_H(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ShiftRightArithmetic(registers->H, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SRA_L(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ShiftRightArithmetic(registers->L, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SRA_mHL(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t val = memory[registers->HL];
	Helpers::ShiftRightArithmetic(val, registers);
	memory.Write(registers->HL, val);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SRA_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ShiftRightArithmetic(registers->A, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SWAP_B(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SwapNibbles(registers->B, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SWAP_C(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SwapNibbles(registers->C, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SWAP_D(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SwapNibbles(registers->D, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SWAP_E(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SwapNibbles(registers->E, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SWAP_H(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SwapNibbles(registers->H, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SWAP_L(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SwapNibbles(registers->L, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SWAP_mHL(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t val = memory[registers->HL];
	Helpers::SwapNibbles(val, registers);
	memory.Write(registers->HL, val);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SWAP_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SwapNibbles(registers->A, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SRL_B(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ShiftRightLogic(registers->B, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SRL_C(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ShiftRightLogic(registers->C, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SRL_D(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ShiftRightLogic(registers->D, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SRL_E(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ShiftRightLogic(registers->E, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SRL_H(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ShiftRightLogic(registers->H, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SRL_L(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ShiftRightLogic(registers->L, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SRL_mHL(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t val = memory[registers->HL];
	Helpers::ShiftRightLogic(val, registers);
	memory.Write(registers->HL, val);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SRL_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ShiftRightLogic(registers->A, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_0_B(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->B, 0, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_0_C(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->C, 0, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_0_D(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->D, 0, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_0_E(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->E, 0, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_0_H(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->H, 0, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_0_L(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->L, 0, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_0_mHL(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(memory[registers->HL], 0, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_0_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->A, 0, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_1_B(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->B, 1, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_1_C(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->C, 1, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_1_D(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->D, 1, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_1_E(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->E, 1, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_1_H(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->H, 1, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_1_L(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->L, 1, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_1_mHL(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(memory[registers->HL], 1, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_1_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->A, 1, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_2_B(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->B, 2, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_2_C(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->C, 2, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_2_D(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->D, 2, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_2_E(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->E, 2, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_2_H(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->H, 2, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_2_L(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->L, 2, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_2_mHL(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(memory[registers->HL], 2, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_2_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->A, 2, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_3_B(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->B, 3, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_3_C(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->C, 3, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_3_D(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->D, 3, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_3_E(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->E, 3, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_3_H(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->H, 3, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_3_L(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->L, 3, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_3_mHL(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(memory[registers->HL], 3, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_3_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->A, 3, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_4_B(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->B, 4, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_4_C(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->C, 4, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_4_D(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->D, 4, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_4_E(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->E, 4, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_4_H(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->H, 4, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_4_L(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->L, 4, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_4_mHL(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(memory[registers->HL], 4, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_4_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->A, 4, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_5_B(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->B, 5, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_5_C(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->C, 5, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_5_D(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->D, 5, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_5_E(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->E, 5, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_5_H(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->H, 5, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_5_L(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->L, 5, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_5_mHL(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(memory[registers->HL], 5, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_5_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->A, 5, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_6_B(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->B, 6, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_6_C(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->C, 6, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_6_D(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->D, 6, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_6_E(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->E, 6, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_6_H(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->H, 6, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_6_L(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->L, 6, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_6_mHL(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(memory[registers->HL], 6, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_6_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->A, 6, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_7_B(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->B, 7, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_7_C(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->C, 7, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_7_D(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->D, 7, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_7_E(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->E, 7, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_7_H(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->H, 7, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_7_L(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->L, 7, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_7_mHL(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(memory[registers->HL], 7, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::BIT_7_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->A, 7, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_0_B(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->B, 0, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_0_C(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->C, 0, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_0_D(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->D, 0, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_0_E(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->E, 0, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_0_H(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->H, 0, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_0_L(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->L, 0, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_0_mHL(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t val = memory[registers->HL];
	Helpers::ResetBit(val, 0, registers);
	memory.Write(registers->HL, val);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_0_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->A, 0, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_1_B(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->B, 1, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_1_C(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->C, 1, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_1_D(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->D, 1, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_1_E(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->E, 1, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_1_H(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->H, 1, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_1_L(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->L, 1, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_1_mHL(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t val = memory[registers->HL];
	Helpers::ResetBit(val, 1, registers);
	memory.Write(registers->HL, val);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_1_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->A, 1, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_2_B(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->B, 2, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_2_C(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->C, 2, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_2_D(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->D, 2, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_2_E(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->E, 2, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_2_H(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->H, 2, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_2_L(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->L, 2, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_2_mHL(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t val = memory[registers->HL];
	Helpers::ResetBit(val, 2, registers);
	memory.Write(registers->HL, val);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_2_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->A, 2, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_3_B(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->B, 3, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_3_C(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->C, 3, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_3_D(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->D, 3, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_3_E(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->E, 3, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_3_H(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->H, 3, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_3_L(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->L, 3, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_3_mHL(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t val = memory[registers->HL];
	Helpers::ResetBit(val, 3, registers);
	memory.Write(registers->HL, val);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_3_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->A, 3, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_4_B(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->B, 4, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_4_C(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->C, 4, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_4_D(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->D, 4, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_4_E(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->E, 4, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_4_H(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->H, 4, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_4_L(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->L, 4, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_4_mHL(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t val = memory[registers->HL];
	Helpers::ResetBit(val, 4, registers);
	memory.Write(registers->HL, val);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_4_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->A, 4, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_5_B(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->B, 5, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_5_C(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->C, 5, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_5_D(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->D, 5, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_5_E(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->E, 5, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_5_H(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->H, 5, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_5_L(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->L, 5, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_5_mHL(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t val = memory[registers->HL];
	Helpers::ResetBit(val, 5, registers);
	memory.Write(registers->HL, val);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_5_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->A, 5, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_6_B(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->B, 6, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_6_C(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->C, 6, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_6_D(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->D, 6, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_6_E(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->E, 6, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_6_H(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->H, 6, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_6_L(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->L, 6, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_6_mHL(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t val = memory[registers->HL];
	Helpers::ResetBit(val, 6, registers);
	memory.Write(registers->HL, val);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_6_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->A, 6, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_7_B(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->B, 7, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_7_C(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->C, 7, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_7_D(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->D, 7, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_7_E(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->E, 7, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_7_H(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->H, 7, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_7_L(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->L, 7, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_7_mHL(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t val = memory[registers->HL];
	Helpers::ResetBit(val, 7, registers);
	memory.Write(registers->HL, val);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::RES_7_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->A, 7, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_0_B(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->B, 0, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_0_C(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->C, 0, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_0_D(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->D, 0, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_0_E(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->E, 0, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_0_H(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->H, 0, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_0_L(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->L, 0, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_0_mHL(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t val = memory[registers->HL];
	Helpers::SetBit(val, 0, registers);
	memory.Write(registers->HL, val);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_0_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->A, 0, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_1_B(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->B, 1, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_1_C(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->C, 1, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_1_D(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->D, 1, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_1_E(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->E, 1, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_1_H(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->H, 1, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_1_L(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->L, 1, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_1_mHL(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t val = memory[registers->HL];
	Helpers::SetBit(val, 1, registers);
	memory.Write(registers->HL, val);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_1_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->A, 1, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_2_B(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->B, 2, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_2_C(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->C, 2, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_2_D(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->D, 2, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_2_E(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->E, 2, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_2_H(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->H, 2, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_2_L(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->L, 2, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_2_mHL(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t val = memory[registers->HL];
	Helpers::SetBit(val, 2, registers);
	memory.Write(registers->HL, val);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_2_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->A, 2, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_3_B(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->B, 3, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_3_C(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->C, 3, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_3_D(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->D, 3, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_3_E(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->E, 3, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_3_H(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->H, 3, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_3_L(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->L, 3, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_3_mHL(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t val = memory[registers->HL];
	Helpers::SetBit(val, 3, registers);
	memory.Write(registers->HL, val);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_3_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->A, 3, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_4_B(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->B, 4, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_4_C(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->C, 4, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_4_D(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->D, 4, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_4_E(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->E, 4, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_4_H(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->H, 4, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_4_L(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->L, 4, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_4_mHL(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t val = memory[registers->HL];
	Helpers::SetBit(val, 4, registers);
	memory.Write(registers->HL, val);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_4_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->A, 4, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_5_B(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->B, 5, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_5_C(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->C, 5, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_5_D(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->D, 5, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_5_E(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->E, 5, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_5_H(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->H, 5, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_5_L(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->L, 5, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_5_mHL(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t val = memory[registers->HL];
	Helpers::SetBit(val, 5, registers);
	memory.Write(registers->HL, val);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_5_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->A, 5, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_6_B(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->B, 6, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_6_C(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->C, 6, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_6_D(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->D, 6, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_6_E(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->E, 6, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_6_H(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->H, 6, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_6_L(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->L, 6, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_6_mHL(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t val = memory[registers->HL];
	Helpers::SetBit(val, 6, registers);
	memory.Write(registers->HL, val);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_6_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->A, 6, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_7_B(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->B, 7, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_7_C(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->C, 7, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_7_D(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->D, 7, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_7_E(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->E, 7, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_7_H(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->H, 7, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_7_L(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->L, 7, registers);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_7_mHL(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint8_t val = memory[registers->HL];
	Helpers::SetBit(val, 7, registers);
	memory.Write(registers->HL, val);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::SET_7_A(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->A, 7, registers);
	LOG_INSTRUCTION(mnemonic);
	return 0;
}

//16 bit loads 
//-------------------------------------------------------------------------------------------------

uint32_t InstructionFunctions::LD_BC_nn(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint16_t immediate = Helpers::Read16Bit(registers->PC, memory);
	registers->BC = immediate;
	LOG_INSTRUCTION(mnemonic, immediate);
    return 0;
}

uint32_t InstructionFunctions::LD_DE_nn(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint16_t immediate = Helpers::Read16Bit(registers->PC, memory);
	registers->DE = immediate;
	LOG_INSTRUCTION(mnemonic, immediate);
    return 0;
}

uint32_t InstructionFunctions::LD_HL_nn(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint16_t immediate = Helpers::Read16Bit(registers->PC, memory);
	registers->HL = immediate;
	LOG_INSTRUCTION(mnemonic, immediate);
    return 0;
}

uint32_t InstructionFunctions::LD_SP_nn(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint16_t immediate = Helpers::Read16Bit(registers->PC, memory);
	registers->SP = immediate;
	LOG_INSTRUCTION(mnemonic, immediate);
    return 0;
}

uint32_t InstructionFunctions::LD_mnn_SP(const char* mnemonic, Registers* registers, Memory& memory)
{
	uint16_t immediate = Helpers::Read16Bit(registers->PC, memory);
	Helpers::Write16Bit(registers->SP, immediate, memory);
	LOG_INSTRUCTION(mnemonic, immediate);
    return 0;
}

uint32_t InstructionFunctions::LD_HL_SP_n(const char* mnemonic, Registers* registers, Memory& memory)
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
    return 0;
}

uint32_t InstructionFunctions::LD_SP_HL(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->SP = registers->HL;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::POP_BC(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->BC = Helpers::Read16Bit(registers->SP, memory);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::POP_DE(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->DE = Helpers::Read16Bit(registers->SP, memory);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::POP_HL(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->HL = Helpers::Read16Bit(registers->SP, memory);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::POP_AF(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->SetAllFlags(memory[registers->SP++]);
	registers->A = memory[registers->SP++];
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::PUSH_BC(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->SP -= 2;
	Helpers::Write16Bit(registers->BC, registers->SP, memory);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::PUSH_DE(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->SP -= 2;
	Helpers::Write16Bit(registers->DE, registers->SP, memory);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::PUSH_HL(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->SP -= 2;
	Helpers::Write16Bit(registers->HL, registers->SP, memory);
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::PUSH_AF(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->SP -= 2;
	Helpers::Write16Bit(registers->AF, registers->SP, memory);
	LOG_INSTRUCTION(mnemonic);
	return 0;
}

//16 bit arithmatic/logic 
//-------------------------------------------------------------------------------------------------
uint32_t InstructionFunctions::INC_BC(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->BC++;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::INC_DE(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->DE++;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::INC_HL(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->HL++;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::INC_SP(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->SP++;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::ADD_HL_BC(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetFlags16(registers->HL, registers->BC, registers);
	registers->HL += registers->BC;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::ADD_HL_DE(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetFlags16(registers->HL, registers->DE, registers);
	registers->HL += registers->DE;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::ADD_HL_HL(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetFlags16(registers->HL, registers->HL, registers);
	registers->HL += registers->HL;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::ADD_HL_SP(const char* mnemonic, Registers* registers, Memory& memory)
{
	Helpers::SetFlags16(registers->HL, registers->SP, registers);
	registers->HL += registers->SP;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::ADD_SP_n(const char* mnemonic, Registers* registers, Memory& memory)
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
    return 0;
}

uint32_t InstructionFunctions::DEC_BC(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->BC--;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::DEC_DE(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->DE--;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::DEC_HL(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->HL--;
	LOG_INSTRUCTION(mnemonic);
    return 0;
}

uint32_t InstructionFunctions::DEC_SP(const char* mnemonic, Registers* registers, Memory& memory)
{
	registers->SP--;
	LOG_INSTRUCTION(mnemonic);
	return 0;
}
