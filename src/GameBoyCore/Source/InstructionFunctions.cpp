#include "InstructionFunctions.h"
#include "Helpers.h"
#include <cassert>
#include "Interrupts.h"

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

		FORCE_INLINE void Write16BitMSB(uint16_t data, uint16_t addr, Memory& memory)
		{
			uint8_t msb = static_cast<uint8_t>(data >> 8);
			memory.Write(addr, msb);
		}

		FORCE_INLINE void Write16BitLSB(uint16_t data, uint16_t addr, Memory& memory)
		{
			uint8_t lsb = static_cast<uint8_t>(data);
			memory.Write(addr, lsb);
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
			bool halfCarry = !subtract && ((static_cast<uint16_t>(previousReg) & 0xF) + (operand & 0xF) > 0xF) || subtract && ((static_cast<int16_t>(previousReg & 0xF) - static_cast<int16_t>(operand & 0xF)) & 0x10) > 0;
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
			Helpers::Subtraction(operand1, cy, registers);
			uint8_t tmpCy = registers->GetFlag(Registers::Flags::cy);
			uint8_t tmpH = registers->GetFlag(Registers::Flags::h);
			Helpers::Subtraction(operand1, operand2, registers);
			registers->OrFlag(Registers::Flags::cy, tmpCy);
			registers->OrFlag(Registers::Flags::h, tmpH);
		}

		FORCE_INLINE void AdditionWithCarry(uint8_t& operand1, uint8_t operand2, Registers* registers)
		{
			uint8_t cy = registers->GetFlag(Registers::Flags::cy);
			Helpers::Addition(operand1, cy, registers);
			uint8_t tmpCy = registers->GetFlag(Registers::Flags::cy);
			uint8_t tmpH = registers->GetFlag(Registers::Flags::h);
			Helpers::Addition(operand1, operand2, registers);
			registers->OrFlag(Registers::Flags::cy, tmpCy);
			registers->OrFlag(Registers::Flags::h, tmpH);
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

		FORCE_INLINE InstructionResult Call(uint16_t addr, uint8_t cycle, Registers* registers, Memory& memory)
		{
			switch (cycle)
			{
			case 0:
				registers->SP--;
				return InstructionResult::Continue;
			case 1:
				Write16BitMSB(registers->PC, registers->SP, memory);
				registers->SP--;
				return InstructionResult::Continue;
			case 2:
				Write16BitLSB(registers->PC, registers->SP, memory);
				registers->PC = addr;
				return InstructionResult::Continue;
			case 3:
				return InstructionResult::Finished;
			}
			LOG_ERROR("Invalid cycle count");
			return InstructionResult::Finished;
		}

		FORCE_INLINE InstructionResult Push(uint16_t value, uint8_t cycle, Registers* registers, Memory& memory)
		{
			switch (cycle)
			{
			case 0:
				registers->SP--;
				return InstructionResult::Continue;
			case 1:
				Write16BitMSB(value, registers->SP, memory);
				registers->SP--;
				return InstructionResult::Continue;
			case 2:
				Write16BitLSB(value, registers->SP, memory);
				return InstructionResult::Continue;
			case 3:
				return InstructionResult::Finished;
			}
			LOG_ERROR("Invalid cycle count");
			return InstructionResult::Finished;
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



InstructionResult InstructionFunctions::UNIMPLEMENTED(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	LOG_ERROR("Instruction not implemented");
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::NOP(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SCF(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->ResetFlag(Registers::Flags::h);
	registers->ResetFlag(Registers::Flags::n);
	registers->SetFlag(Registers::Flags::cy);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::CCF(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->ResetFlag(Registers::Flags::h);
	registers->ResetFlag(Registers::Flags::n);
	registers->SetFlag(Registers::Flags::cy, !registers->IsFlagSet(Registers::Flags::cy));
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::JR_NZ_n(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_s8 = memory[registers->PC++];
		return InstructionResult::Continue;
	}

	if (data.m_cycles == 1 && !registers->IsFlagSet(Registers::Flags::zf))
	{
		registers->PC += data.m_tmp_s8;
		return InstructionResult::Continue;
	}

	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::JR_Z_n(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_s8 = memory[registers->PC++];
		return InstructionResult::Continue;
	}

	if (data.m_cycles == 1 && registers->IsFlagSet(Registers::Flags::zf))
	{
		registers->PC += data.m_tmp_s8;
		return InstructionResult::Continue;
	}

	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::JR_NC_n(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_s8 = memory[registers->PC++];
		return InstructionResult::Continue;
	}

	if (data.m_cycles == 1 && !registers->IsFlagSet(Registers::Flags::cy))
	{
		registers->PC += data.m_tmp_s8;
		return InstructionResult::Continue;
	}

	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::JR_C_n(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_s8 = memory[registers->PC++];
		return InstructionResult::Continue;
	}
	
	if (data.m_cycles == 1 && registers->IsFlagSet(Registers::Flags::cy))
	{
		registers->PC += data.m_tmp_s8;
		return InstructionResult::Continue;
	}

	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::JR_n(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_s8 = memory[registers->PC++];
		return InstructionResult::Continue;
	}
	else if (data.m_cycles == 1)
	{
		registers->PC += data.m_tmp_s8;
		return InstructionResult::Continue;
	}

	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::JP_NZ_nn(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		//TODO needs to be split up?
		data.m_tmp_16 = Helpers::Read16Bit(registers->PC, memory);
		return InstructionResult::Delay_2;
	}

	if (data.m_cycles == 2 && !registers->IsFlagSet(Registers::Flags::zf))
	{
		registers->PC = data.m_tmp_16;
		return InstructionResult::Continue;
	}
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::JP_Z_nn(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		//TODO needs to be split up?
		data.m_tmp_16 = Helpers::Read16Bit(registers->PC, memory);
		return InstructionResult::Delay_2;
	}

	if (data.m_cycles == 2 && registers->IsFlagSet(Registers::Flags::zf))
	{
		registers->PC = data.m_tmp_16;
		return InstructionResult::Continue;
	}
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::JP_NC_nn(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		//TODO needs to be split up?
		data.m_tmp_16 = Helpers::Read16Bit(registers->PC, memory);
		return InstructionResult::Delay_2;
	}

	if (data.m_cycles == 2 && !registers->IsFlagSet(Registers::Flags::cy))
	{
		registers->PC = data.m_tmp_16;
		return InstructionResult::Continue;
	}
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::JP_C_nn(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		//TODO needs to be split up?
		data.m_tmp_16 = Helpers::Read16Bit(registers->PC, memory);
		return InstructionResult::Delay_2;
	}
	
	if (data.m_cycles == 2 && registers->IsFlagSet(Registers::Flags::cy))
	{
		registers->PC = data.m_tmp_16;
		return InstructionResult::Continue;
	}
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::JP_nn(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		//TODO needs to be split up?
		data.m_tmp_16 = Helpers::Read16Bit(registers->PC, memory);
		return InstructionResult::Delay_2;
	}

	if (data.m_cycles == 2)
	{
		registers->PC = data.m_tmp_16;
		return InstructionResult::Continue;
	}
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::JP_HL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->PC = registers->HL;
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::CALL_NZ_nn(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		//TODO needs to be split up?
		data.m_tmp_16 = Helpers::Read16Bit(registers->PC, memory);
		return InstructionResult::Delay_2;
	}

	if (!registers->IsFlagSet(Registers::Flags::zf))
	{
		return Helpers::Call(data.m_tmp_16, data.m_cycles - 2, registers, memory);
	}
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::CALL_Z_nn(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		//TODO needs to be split up?
		data.m_tmp_16 = Helpers::Read16Bit(registers->PC, memory);
		return InstructionResult::Delay_2;
	}

	if (registers->IsFlagSet(Registers::Flags::zf))
	{
		return Helpers::Call(data.m_tmp_16, data.m_cycles - 2, registers, memory);
	}
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::CALL_NC_nn(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		//TODO needs to be split up?
		data.m_tmp_16 = Helpers::Read16Bit(registers->PC, memory);
		return InstructionResult::Delay_2;
	}

	if (!registers->IsFlagSet(Registers::Flags::cy))
	{
		return Helpers::Call(data.m_tmp_16, data.m_cycles - 2, registers, memory);
	}
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::CALL_C_nn(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		//TODO needs to be split up?
		data.m_tmp_16 = Helpers::Read16Bit(registers->PC, memory);
		return InstructionResult::Delay_2;
	}

	if (registers->IsFlagSet(Registers::Flags::cy))
	{
		return Helpers::Call(data.m_tmp_16, data.m_cycles - 2, registers, memory);
	}
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::CALL_nn(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		//TODO needs to be split up?
		data.m_tmp_16 = Helpers::Read16Bit(registers->PC, memory);
		return InstructionResult::Delay_2;
	}

	return Helpers::Call(data.m_tmp_16, data.m_cycles - 2, registers, memory);
}

InstructionResult InstructionFunctions::RST_00(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	return Helpers::Call(0x00, data.m_cycles, registers, memory);
}

InstructionResult InstructionFunctions::RST_10(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	return Helpers::Call(0x10, data.m_cycles, registers, memory);
}

InstructionResult InstructionFunctions::RST_20(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	return Helpers::Call(0x20, data.m_cycles, registers, memory);
}

InstructionResult InstructionFunctions::RST_30(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	return Helpers::Call(0x30, data.m_cycles, registers, memory);
}

InstructionResult InstructionFunctions::RST_08(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	return Helpers::Call(0x08, data.m_cycles, registers, memory);
}

InstructionResult InstructionFunctions::RST_18(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	return Helpers::Call(0x18, data.m_cycles, registers, memory);
}

InstructionResult InstructionFunctions::RST_28(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	return Helpers::Call(0x28, data.m_cycles, registers, memory);
}

InstructionResult InstructionFunctions::RST_38(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	return Helpers::Call(0x38, data.m_cycles, registers, memory);
}

InstructionResult InstructionFunctions::RET_NZ(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		return InstructionResult::Continue;
	}

	if (data.m_cycles == 1 && !registers->IsFlagSet(Registers::Flags::zf))
	{
		//TODO needs to be split up?
		registers->PC = Helpers::Read16Bit(registers->SP, memory);
		return InstructionResult::Delay_3;
	}
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RET_Z(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		return InstructionResult::Continue;
	}
	
	if (data.m_cycles == 1 && registers->IsFlagSet(Registers::Flags::zf))
	{
		registers->PC = Helpers::Read16Bit(registers->SP, memory);
		return InstructionResult::Delay_3;
	}
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RET_NC(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		return InstructionResult::Continue;
	}
	
	if (data.m_cycles == 1 && !registers->IsFlagSet(Registers::Flags::cy))
	{
		registers->PC = Helpers::Read16Bit(registers->SP, memory);
		return InstructionResult::Delay_3;
	}
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RET_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		return InstructionResult::Continue;
	}
	
	if (data.m_cycles == 1 && registers->IsFlagSet(Registers::Flags::cy))
	{
		registers->PC = Helpers::Read16Bit(registers->SP, memory);
		return InstructionResult::Delay_3;
	}
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RET(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		registers->PC = Helpers::Read16Bit(registers->SP, memory);
		return InstructionResult::Delay_3;
	}
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RETI(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		registers->PC = Helpers::Read16Bit(registers->SP, memory);
		registers->IMEF = true;
		return InstructionResult::Delay_3;
	}

	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::EI(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->IMEF = true;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::DI(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->IMEF = false;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::HALT(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->CpuState = Registers::State::Halt;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::STOP(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->PC++;
	registers->CpuState = Registers::State::Stop;
	memory.Write(0xFF04, 0); // Reset divider
	
	return InstructionResult::Finished;
}


//8 bit loads 
//-------------------------------------------------------------------------------------------------
InstructionResult InstructionFunctions::LD_mBC_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
 	if (data.m_cycles == 0)
	{
		memory.Write(registers->BC, registers->A);
		return InstructionResult::Continue;
	}

	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_mDE_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		memory.Write(registers->DE, registers->A);
		return InstructionResult::Continue;
	}
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_mHLinc_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		memory.Write(registers->HL++, registers->A);
		return InstructionResult::Continue;
	}
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_mHLdec_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		memory.Write(registers->HL--, registers->A);
		return InstructionResult::Continue;
	}
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_B_n(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->PC++];
		return InstructionResult::Continue;
	}

	registers->B = data.m_tmp_u8;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_D_n(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->PC++];
		return InstructionResult::Continue;
	}

	registers->D = data.m_tmp_u8;

	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_H_n(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->PC++];
		return InstructionResult::Continue;
	}

	registers->H = data.m_tmp_u8;

	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_mHL_n(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->PC++];
		return InstructionResult::Continue;
	}
	else if (data.m_cycles == 1)
	{
		memory.Write(registers->HL, data.m_tmp_u8);
		return InstructionResult::Continue;
	}
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_C_n(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->PC++];
		return InstructionResult::Continue;
	}

	registers->C = data.m_tmp_u8;

	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_E_n(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->PC++];
		return InstructionResult::Continue;
	}

	registers->E = data.m_tmp_u8;

	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_L_n(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->PC++];
		return InstructionResult::Continue;
	}

	registers->L = data.m_tmp_u8;

	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_A_n(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->PC++];
		return InstructionResult::Continue;
	}

	registers->A = data.m_tmp_u8;

	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_B_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->B = registers->B;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_B_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->B = registers->C;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_B_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->B = registers->D;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_B_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->B = registers->E;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_B_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->B = registers->H;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_B_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->B = registers->L;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_B_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->HL];
		return InstructionResult::Continue;
	}

	registers->B = data.m_tmp_u8;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_B_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->B = registers->A;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_C_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->C = registers->B;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_C_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->C = registers->C;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_C_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->C = registers->D;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_C_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->C = registers->E;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_C_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->C = registers->H;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_C_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->C = registers->L;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_C_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->HL];
		return InstructionResult::Continue;
	}

	registers->C = data.m_tmp_u8;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_C_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->C = registers->A;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_D_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->D = registers->B;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_D_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->D = registers->C;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_D_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->D = registers->D;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_D_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->D = registers->E;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_D_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->D = registers->H;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_D_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->D = registers->L;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_D_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->HL];
		return InstructionResult::Continue;
	}

	registers->D = data.m_tmp_u8;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_D_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->D = registers->A;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_E_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->E = registers->B;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_E_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->E = registers->C;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_E_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->E = registers->D;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_E_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->E = registers->E;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_E_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->E = registers->H;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_E_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->E = registers->L;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_E_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->HL];
		return InstructionResult::Continue;
	}

	registers->E = data.m_tmp_u8;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_E_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->E = registers->A;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_H_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->H = registers->B;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_H_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->H = registers->C;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_H_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->H = registers->D;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_H_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->H = registers->E;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_H_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->H = registers->H;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_H_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->H = registers->L;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_H_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->HL];
		return InstructionResult::Continue;
	}

	registers->H = data.m_tmp_u8;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_H_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->H = registers->A;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_L_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->L = registers->B;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_L_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->L = registers->C;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_L_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->L = registers->D;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_L_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->L = registers->E;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_L_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->L = registers->H;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_L_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->L = registers->L;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_L_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->HL];
		return InstructionResult::Continue;
	}

	registers->L = data.m_tmp_u8;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_L_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->L = registers->A;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_mHL_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		memory.Write(registers->HL, registers->B);
		return InstructionResult::Continue;
	}
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_mHL_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		memory.Write(registers->HL, registers->C);
		return InstructionResult::Continue;
	}
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_mHL_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		memory.Write(registers->HL, registers->D);
		return InstructionResult::Continue;
	}
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_mHL_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		memory.Write(registers->HL, registers->E);
		return InstructionResult::Continue;
	}
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_mHL_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		memory.Write(registers->HL, registers->H);
		return InstructionResult::Continue;
	}
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_mHL_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		memory.Write(registers->HL, registers->L);
		return InstructionResult::Continue;
	}
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_mHL_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		memory.Write(registers->HL, registers->A);
		return InstructionResult::Continue;
	}
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_A_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->A = registers->B;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_A_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->A = registers->C;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_A_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->A = registers->D;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_A_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->A = registers->E;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_A_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->A = registers->H;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_A_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->A = registers->L;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_A_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->HL];
		return InstructionResult::Continue;
	}

	registers->A = data.m_tmp_u8;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_A_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->A = registers->A;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_A_mBC(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->BC];
		return InstructionResult::Continue;
	}

	registers->A = data.m_tmp_u8;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_A_mDE(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->DE];
		return InstructionResult::Continue;
	}

	registers->A = data.m_tmp_u8;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_A_mHLinc(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->HL++];
		return InstructionResult::Continue;
	}

	registers->A = data.m_tmp_u8;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_A_mHLdec(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->HL--];
		return InstructionResult::Continue;
	}

	registers->A = data.m_tmp_u8;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LDH_mn_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->PC++];
		return InstructionResult::Continue;
	}
	else if (data.m_cycles == 1)
	{
		uint16_t addr = Helpers::u16(data.m_tmp_u8, 0xFF);
		memory.Write(addr, registers->A);
		return InstructionResult::Continue;
	}
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LDH_A_mn(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->PC++];
		return InstructionResult::Continue;
	}
	else if (data.m_cycles == 1)
	{
		uint16_t addr = Helpers::u16(data.m_tmp_u8, 0xFF);
		registers->A = memory[addr];
		return InstructionResult::Continue;
	}
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LDH_mC_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		uint16_t addr = Helpers::u16(registers->C, 0xFF);
		memory.Write(addr, registers->A);
		return InstructionResult::Continue;
	}
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LDH_A_mC(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		uint16_t addr = Helpers::u16(registers->C, 0xFF);
		registers->A = memory[addr];
		return InstructionResult::Continue;
	}
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_mnn_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		//TODO split this up?
		data.m_tmp_16 = Helpers::Read16Bit(registers->PC, memory);
		return InstructionResult::Delay_2;
	}
	else if (data.m_cycles == 2)
	{
		memory.Write(data.m_tmp_16, registers->A);
		return InstructionResult::Continue;
	}
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_A_mnn(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		//TODO split this up?
		data.m_tmp_16 = Helpers::Read16Bit(registers->PC, memory);
		return InstructionResult::Delay_2;
	}
	else if (data.m_cycles == 2)
	{
		registers->A = memory[data.m_tmp_16];
		return InstructionResult::Continue;
	}
	
	return InstructionResult::Finished;
}

//8 bit arithmatic/logic 
//-------------------------------------------------------------------------------------------------
InstructionResult InstructionFunctions::INC_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	uint8_t& reg = registers->B;
	uint8_t prevReg = reg;
	reg++;
	Helpers::SetFlagsNoCarry(prevReg, 1, reg, false, registers);

	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::INC_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	uint8_t& reg = registers->D;
	uint8_t prevReg = reg;
	reg++;
	Helpers::SetFlagsNoCarry(prevReg, 1, reg, false, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::INC_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	uint8_t& reg = registers->H;
	uint8_t prevReg = reg;
	reg++;
	Helpers::SetFlagsNoCarry(prevReg, 1, reg, false, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::INC_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->HL];
		return InstructionResult::Continue;
	}
	else if (data.m_cycles == 1)
	{
		uint8_t reg = data.m_tmp_u8 + 1;
		memory.Write(registers->HL, reg);
		Helpers::SetFlagsNoCarry(data.m_tmp_u8, 1, reg, false, registers);
		return InstructionResult::Continue;
	}
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::INC_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	uint8_t& reg = registers->C;
	uint8_t prevReg = reg;
	reg++;
	Helpers::SetFlagsNoCarry(prevReg, 1, reg, false, registers);

	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::INC_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	uint8_t& reg = registers->E;
	uint8_t prevReg = reg;
	reg++;
	Helpers::SetFlagsNoCarry(prevReg, 1, reg, false, registers);

	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::INC_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	uint8_t& reg = registers->L;
	uint8_t prevReg = reg;
	reg++;
	Helpers::SetFlagsNoCarry(prevReg, 1, reg, false, registers);

	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::INC_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	uint8_t& reg = registers->A;
	uint8_t prevReg = reg;
	reg++;
	Helpers::SetFlagsNoCarry(prevReg, 1, reg, false, registers);

	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::DEC_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	uint8_t& reg = registers->B;
	uint8_t prevReg = reg;
	reg--;
	Helpers::SetFlagsNoCarry(prevReg, 1, reg, true, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::DEC_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	uint8_t& reg = registers->D;
	uint8_t prevReg = reg;
	reg--;
	Helpers::SetFlagsNoCarry(prevReg, 1, reg, true, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::DEC_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	uint8_t& reg = registers->H;
	uint8_t prevReg = reg;
	reg--;
	Helpers::SetFlagsNoCarry(prevReg, 1, reg, true, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::DEC_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{	 
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->HL];
		return InstructionResult::Continue;
	}
	else if (data.m_cycles == 1)
	{
		uint8_t reg = data.m_tmp_u8 - 1;
		memory.Write(registers->HL, reg);
		Helpers::SetFlagsNoCarry(data.m_tmp_u8, 1, reg, true, registers);
		return InstructionResult::Continue;
	}
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::DEC_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	uint8_t& reg = registers->C;
	uint8_t prevReg = reg;
	reg--;
	Helpers::SetFlagsNoCarry(prevReg, 1, reg, true, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::DEC_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	uint8_t& reg = registers->E;
	uint8_t prevReg = reg;
	reg--;
	Helpers::SetFlagsNoCarry(prevReg, 1, reg, true, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::DEC_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	uint8_t& reg = registers->L;
	uint8_t prevReg = reg;
	reg--;
	Helpers::SetFlagsNoCarry(prevReg, 1, reg, true, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::DEC_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	uint8_t& reg = registers->A;
	uint8_t prevReg = reg;
	reg--;
	Helpers::SetFlagsNoCarry(prevReg, 1, reg, true, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::CPL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	registers->A = ~registers->A;
	registers->SetFlag(Registers::Flags::n);
	registers->SetFlag(Registers::Flags::h);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::ADD_A_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::Addition(registers->A, registers->B, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::ADD_A_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::Addition(registers->A, registers->C, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::ADD_A_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::Addition(registers->A, registers->D, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::ADD_A_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::Addition(registers->A, registers->E, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::ADD_A_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::Addition(registers->A, registers->H, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::ADD_A_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::Addition(registers->A, registers->L, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::ADD_A_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
 	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->HL];
		return InstructionResult::Continue;
	}

	Helpers::Addition(registers->A, data.m_tmp_u8, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::ADD_A_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::Addition(registers->A, registers->A, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::ADC_A_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::AdditionWithCarry(registers->A, registers->B, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::ADC_A_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::AdditionWithCarry(registers->A, registers->C, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::ADC_A_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::AdditionWithCarry(registers->A, registers->D, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::ADC_A_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::AdditionWithCarry(registers->A, registers->E, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::ADC_A_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::AdditionWithCarry(registers->A, registers->H, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::ADC_A_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::AdditionWithCarry(registers->A, registers->L, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::ADC_A_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->HL];
		return InstructionResult::Continue;
	}

	Helpers::AdditionWithCarry(registers->A, data.m_tmp_u8, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::ADC_A_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::AdditionWithCarry(registers->A, registers->A, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SUB_A_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::Subtraction(registers->A, registers->B, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SUB_A_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::Subtraction(registers->A, registers->C, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SUB_A_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::Subtraction(registers->A, registers->D, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SUB_A_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::Subtraction(registers->A, registers->E, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SUB_A_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::Subtraction(registers->A, registers->H, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SUB_A_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::Subtraction(registers->A, registers->L, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SUB_A_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->HL];
		return InstructionResult::Continue;
	}

	Helpers::Subtraction(registers->A, data.m_tmp_u8, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SUB_A_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::Subtraction(registers->A, registers->A, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SBC_A_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SubtractionWithCarry(registers->A, registers->B, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SBC_A_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SubtractionWithCarry(registers->A, registers->C, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SBC_A_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SubtractionWithCarry(registers->A, registers->D, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SBC_A_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SubtractionWithCarry(registers->A, registers->E, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SBC_A_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SubtractionWithCarry(registers->A, registers->H, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SBC_A_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SubtractionWithCarry(registers->A, registers->L, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SBC_A_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->HL];
		return InstructionResult::Continue;
	}

	Helpers::SubtractionWithCarry(registers->A, data.m_tmp_u8, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SBC_A_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SubtractionWithCarry(registers->A, registers->A, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::AND_A_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::BitwiseAnd(registers->A, registers->B, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::AND_A_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::BitwiseAnd(registers->A, registers->C, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::AND_A_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::BitwiseAnd(registers->A, registers->D, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::AND_A_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::BitwiseAnd(registers->A, registers->E, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::AND_A_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::BitwiseAnd(registers->A, registers->H, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::AND_A_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::BitwiseAnd(registers->A, registers->L, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::AND_A_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->HL];
		return InstructionResult::Continue;
	}

	Helpers::BitwiseAnd(registers->A, data.m_tmp_u8, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::AND_A_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::BitwiseAnd(registers->A, registers->A, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::XOR_A_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::BitwiseXor(registers->A, registers->B, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::XOR_A_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::BitwiseXor(registers->A, registers->C, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::XOR_A_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::BitwiseXor(registers->A, registers->D, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::XOR_A_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::BitwiseXor(registers->A, registers->E, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::XOR_A_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::BitwiseXor(registers->A, registers->H, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::XOR_A_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::BitwiseXor(registers->A, registers->L, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::XOR_A_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->HL];
		return InstructionResult::Continue;
	}

	Helpers::BitwiseXor(registers->A, data.m_tmp_u8, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::XOR_A_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::BitwiseXor(registers->A, registers->A, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::OR_A_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::BitwiseOr(registers->A, registers->B, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::OR_A_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::BitwiseOr(registers->A, registers->C, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::OR_A_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::BitwiseOr(registers->A, registers->D, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::OR_A_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::BitwiseOr(registers->A, registers->E, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::OR_A_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::BitwiseOr(registers->A, registers->H, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::OR_A_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::BitwiseOr(registers->A, registers->L, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::OR_A_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->HL];
		return InstructionResult::Continue;
	}

	Helpers::BitwiseOr(registers->A, data.m_tmp_u8, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::OR_A_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::BitwiseOr(registers->A, registers->A, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::CP_A_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::CompareSubtraction(registers->A, registers->B, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::CP_A_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::CompareSubtraction(registers->A, registers->C, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::CP_A_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::CompareSubtraction(registers->A, registers->D, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::CP_A_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::CompareSubtraction(registers->A, registers->E, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::CP_A_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::CompareSubtraction(registers->A, registers->H, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::CP_A_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::CompareSubtraction(registers->A, registers->L, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::CP_A_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->HL];
		return InstructionResult::Continue;
	}

	Helpers::CompareSubtraction(registers->A, data.m_tmp_u8, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::CP_A_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::CompareSubtraction(registers->A, registers->A, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::ADD_A_n(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->PC++];
		return InstructionResult::Continue;
	}

	Helpers::Addition(registers->A, data.m_tmp_u8, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SUB_A_n(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->PC++];
		return InstructionResult::Continue;
	}

	Helpers::Subtraction(registers->A, data.m_tmp_u8, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::AND_A_n(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->PC++];
		return InstructionResult::Continue;
	}

	Helpers::BitwiseAnd(registers->A, data.m_tmp_u8, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::OR_A_n(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->PC++];
		return InstructionResult::Continue;
	}

	Helpers::BitwiseOr(registers->A, data.m_tmp_u8, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::ADC_A_n(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->PC++];
		return InstructionResult::Continue;
	}

	Helpers::AdditionWithCarry(registers->A, data.m_tmp_u8, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SBC_A_n(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->PC++];
		return InstructionResult::Continue;
	}

	Helpers::SubtractionWithCarry(registers->A, data.m_tmp_u8, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::XOR_A_n(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->PC++];
		return InstructionResult::Continue;
	}

	Helpers::BitwiseXor(registers->A, data.m_tmp_u8, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::CP_A_n(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->PC++];
		return InstructionResult::Continue;
	}

	Helpers::CompareSubtraction(registers->A, data.m_tmp_u8, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::DAA(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
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
		
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RLCA(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::RotateLeft(registers->A, registers);
	registers->SetFlag(Registers::Flags::zf, 0);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RLA(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::RotateLeftWithCarry(registers->A, registers);
	registers->SetFlag(Registers::Flags::zf, 0);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RRCA(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::RotateRight(registers->A, registers);
	registers->SetFlag(Registers::Flags::zf, 0);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RRA(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::RotateRightWithCarry(registers->A, registers);
	registers->SetFlag(Registers::Flags::zf, 0);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RLC_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::RotateLeft(registers->B, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RLC_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::RotateLeft(registers->C, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RLC_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::RotateLeft(registers->D, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RLC_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::RotateLeft(registers->E, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RLC_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::RotateLeft(registers->H, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RLC_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::RotateLeft(registers->L, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RLC_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
 	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->HL];
		return InstructionResult::Continue;
	}
	else if (data.m_cycles == 1)
	{
		Helpers::RotateLeft(data.m_tmp_u8, registers);
		memory.Write(registers->HL, data.m_tmp_u8);
		return InstructionResult::Continue;
	}

	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RLC_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::RotateLeft(registers->A, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RRC_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::RotateRight(registers->B, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RRC_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::RotateRight(registers->C, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RRC_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::RotateRight(registers->D, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RRC_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::RotateRight(registers->E, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RRC_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::RotateRight(registers->H, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RRC_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::RotateRight(registers->L, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RRC_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->HL];
		return InstructionResult::Continue;
	}
	else if (data.m_cycles == 1)
	{
		Helpers::RotateRight(data.m_tmp_u8, registers);
		memory.Write(registers->HL, data.m_tmp_u8);
		return InstructionResult::Continue;
	}

	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RRC_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::RotateRight(registers->A, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RL_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::RotateLeftWithCarry(registers->B, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RL_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::RotateLeftWithCarry(registers->C, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RL_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::RotateLeftWithCarry(registers->D, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RL_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::RotateLeftWithCarry(registers->E, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RL_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::RotateLeftWithCarry(registers->H, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RL_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::RotateLeftWithCarry(registers->L, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RL_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->HL];
		return InstructionResult::Continue;
	}
	else if (data.m_cycles == 1)
	{
		Helpers::RotateLeftWithCarry(data.m_tmp_u8, registers);
		memory.Write(registers->HL, data.m_tmp_u8);
		return InstructionResult::Continue;
	}

	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RL_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::RotateLeftWithCarry(registers->A, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RR_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::RotateRightWithCarry(registers->B, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RR_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::RotateRightWithCarry(registers->C, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RR_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::RotateRightWithCarry(registers->D, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RR_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::RotateRightWithCarry(registers->E, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RR_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::RotateRightWithCarry(registers->H, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RR_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::RotateRightWithCarry(registers->L, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RR_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->HL];
		return InstructionResult::Continue;
	}
	else if (data.m_cycles == 1)
	{
		Helpers::RotateRightWithCarry(data.m_tmp_u8, registers);
		memory.Write(registers->HL, data.m_tmp_u8);
		return InstructionResult::Continue;
	}

	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RR_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::RotateRightWithCarry(registers->A, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SLA_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ShiftLeftArithmetic(registers->B, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SLA_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ShiftLeftArithmetic(registers->C, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SLA_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ShiftLeftArithmetic(registers->D, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SLA_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ShiftLeftArithmetic(registers->E, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SLA_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ShiftLeftArithmetic(registers->H, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SLA_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ShiftLeftArithmetic(registers->L, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SLA_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->HL];
		return InstructionResult::Continue;
	}
	else if (data.m_cycles == 1)
	{
		Helpers::ShiftLeftArithmetic(data.m_tmp_u8, registers);
		memory.Write(registers->HL, data.m_tmp_u8);
		return InstructionResult::Continue;
	}

	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SLA_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ShiftLeftArithmetic(registers->A, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SRA_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ShiftRightArithmetic(registers->B, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SRA_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ShiftRightArithmetic(registers->C, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SRA_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ShiftRightArithmetic(registers->D, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SRA_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ShiftRightArithmetic(registers->E, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SRA_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ShiftRightArithmetic(registers->H, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SRA_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ShiftRightArithmetic(registers->L, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SRA_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->HL];
		return InstructionResult::Continue;
	}
	else if (data.m_cycles == 1)
	{
		Helpers::ShiftRightArithmetic(data.m_tmp_u8, registers);
		memory.Write(registers->HL, data.m_tmp_u8);
		return InstructionResult::Continue;
	}

	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SRA_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ShiftRightArithmetic(registers->A, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SWAP_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SwapNibbles(registers->B, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SWAP_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SwapNibbles(registers->C, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SWAP_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SwapNibbles(registers->D, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SWAP_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SwapNibbles(registers->E, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SWAP_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SwapNibbles(registers->H, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SWAP_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SwapNibbles(registers->L, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SWAP_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->HL];
		return InstructionResult::Continue;
	}
	else if (data.m_cycles == 1)
	{
		Helpers::SwapNibbles(data.m_tmp_u8, registers);
		memory.Write(registers->HL, data.m_tmp_u8);
		return InstructionResult::Continue;
	}

	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SWAP_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SwapNibbles(registers->A, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SRL_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ShiftRightLogic(registers->B, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SRL_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ShiftRightLogic(registers->C, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SRL_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ShiftRightLogic(registers->D, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SRL_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ShiftRightLogic(registers->E, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SRL_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ShiftRightLogic(registers->H, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SRL_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ShiftRightLogic(registers->L, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SRL_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->HL];
		return InstructionResult::Continue;
	}
	else if (data.m_cycles == 1)
	{
		Helpers::ShiftRightLogic(data.m_tmp_u8, registers);
		memory.Write(registers->HL, data.m_tmp_u8);
		return InstructionResult::Continue;
	}

	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SRL_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ShiftRightLogic(registers->A, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_0_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->B, 0, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_0_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->C, 0, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_0_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->D, 0, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_0_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->E, 0, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_0_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->H, 0, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_0_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->L, 0, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_0_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->HL];
		Helpers::TestBit(data.m_tmp_u8, 0, registers);
		return InstructionResult::Continue;
	}

	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_0_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->A, 0, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_1_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->B, 1, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_1_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->C, 1, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_1_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->D, 1, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_1_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->E, 1, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_1_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->H, 1, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_1_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->L, 1, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_1_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->HL];
		Helpers::TestBit(data.m_tmp_u8, 1, registers);
		return InstructionResult::Continue;
	}

	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_1_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->A, 1, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_2_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->B, 2, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_2_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->C, 2, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_2_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->D, 2, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_2_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->E, 2, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_2_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->H, 2, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_2_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->L, 2, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_2_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->HL];
		Helpers::TestBit(data.m_tmp_u8, 2, registers);
		return InstructionResult::Continue;
	}

	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_2_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->A, 2, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_3_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->B, 3, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_3_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->C, 3, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_3_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->D, 3, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_3_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->E, 3, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_3_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->H, 3, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_3_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->L, 3, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_3_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->HL];
		Helpers::TestBit(data.m_tmp_u8, 3, registers);
		return InstructionResult::Continue;
	}

	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_3_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->A, 3, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_4_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->B, 4, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_4_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->C, 4, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_4_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->D, 4, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_4_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->E, 4, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_4_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->H, 4, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_4_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->L, 4, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_4_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->HL];
		Helpers::TestBit(data.m_tmp_u8, 4, registers);
		return InstructionResult::Continue;
	}

	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_4_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->A, 4, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_5_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->B, 5, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_5_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->C, 5, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_5_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->D, 5, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_5_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->E, 5, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_5_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->H, 5, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_5_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->L, 5, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_5_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->HL];
		Helpers::TestBit(data.m_tmp_u8, 5, registers);
		return InstructionResult::Continue;
	}

	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_5_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->A, 5, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_6_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->B, 6, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_6_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->C, 6, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_6_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->D, 6, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_6_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->E, 6, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_6_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->H, 6, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_6_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->L, 6, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_6_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->HL];
		Helpers::TestBit(data.m_tmp_u8, 6, registers);
		return InstructionResult::Continue;
	}

	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_6_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->A, 6, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_7_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->B, 7, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_7_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->C, 7, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_7_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->D, 7, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_7_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->E, 7, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_7_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->H, 7, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_7_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->L, 7, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_7_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->HL];
		Helpers::TestBit(data.m_tmp_u8, 7, registers);
		return InstructionResult::Continue;
	}

	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::BIT_7_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::TestBit(registers->A, 7, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_0_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->B, 0, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_0_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->C, 0, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_0_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->D, 0, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_0_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->E, 0, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_0_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->H, 0, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_0_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->L, 0, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_0_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->HL];
		return InstructionResult::Continue;
	}
	else if (data.m_cycles == 1)
	{
		Helpers::ResetBit(data.m_tmp_u8, 0, registers);
		memory.Write(registers->HL, data.m_tmp_u8);
		return InstructionResult::Continue;
	}

	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_0_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->A, 0, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_1_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->B, 1, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_1_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->C, 1, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_1_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->D, 1, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_1_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->E, 1, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_1_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->H, 1, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_1_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->L, 1, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_1_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->HL];
		return InstructionResult::Continue;
	}
	else if (data.m_cycles == 1)
	{
		Helpers::ResetBit(data.m_tmp_u8, 1, registers);
		memory.Write(registers->HL, data.m_tmp_u8);
		return InstructionResult::Continue;
	}

	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_1_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->A, 1, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_2_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->B, 2, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_2_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->C, 2, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_2_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->D, 2, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_2_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->E, 2, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_2_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->H, 2, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_2_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->L, 2, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_2_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->HL];
		return InstructionResult::Continue;
	}
	else if (data.m_cycles == 1)
	{
		Helpers::ResetBit(data.m_tmp_u8, 2, registers);
		memory.Write(registers->HL, data.m_tmp_u8);
		return InstructionResult::Continue;
	}

	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_2_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->A, 2, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_3_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->B, 3, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_3_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->C, 3, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_3_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->D, 3, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_3_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->E, 3, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_3_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->H, 3, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_3_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->L, 3, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_3_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->HL];
		return InstructionResult::Continue;
	}
	else if (data.m_cycles == 1)
	{
		Helpers::ResetBit(data.m_tmp_u8, 3, registers);
		memory.Write(registers->HL, data.m_tmp_u8);
		return InstructionResult::Continue;
	}

	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_3_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->A, 3, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_4_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->B, 4, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_4_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->C, 4, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_4_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->D, 4, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_4_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->E, 4, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_4_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->H, 4, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_4_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->L, 4, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_4_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->HL];
		return InstructionResult::Continue;
	}
	else if (data.m_cycles == 1)
	{
		Helpers::ResetBit(data.m_tmp_u8, 4, registers);
		memory.Write(registers->HL, data.m_tmp_u8);
		return InstructionResult::Continue;
	}

	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_4_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->A, 4, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_5_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->B, 5, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_5_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->C, 5, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_5_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->D, 5, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_5_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->E, 5, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_5_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->H, 5, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_5_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->L, 5, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_5_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->HL];
		return InstructionResult::Continue;
	}
	else if (data.m_cycles == 1)
	{
		Helpers::ResetBit(data.m_tmp_u8, 5, registers);
		memory.Write(registers->HL, data.m_tmp_u8);
		return InstructionResult::Continue;
	}

	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_5_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->A, 5, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_6_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->B, 6, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_6_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->C, 6, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_6_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->D, 6, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_6_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->E, 6, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_6_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->H, 6, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_6_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->L, 6, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_6_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->HL];
		return InstructionResult::Continue;
	}
	else if (data.m_cycles == 1)
	{
		Helpers::ResetBit(data.m_tmp_u8, 6, registers);
		memory.Write(registers->HL, data.m_tmp_u8);
		return InstructionResult::Continue;
	}

	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_6_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->A, 6, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_7_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->B, 7, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_7_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->C, 7, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_7_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->D, 7, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_7_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->E, 7, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_7_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->H, 7, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_7_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->L, 7, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_7_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->HL];
		return InstructionResult::Continue;
	}
	else if (data.m_cycles == 1)
	{
		Helpers::ResetBit(data.m_tmp_u8, 7, registers);
		memory.Write(registers->HL, data.m_tmp_u8);
		return InstructionResult::Continue;
	}

	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::RES_7_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::ResetBit(registers->A, 7, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_0_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->B, 0, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_0_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->C, 0, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_0_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->D, 0, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_0_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->E, 0, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_0_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->H, 0, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_0_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->L, 0, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_0_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->HL];
		return InstructionResult::Continue;
	}
	else if (data.m_cycles == 1)
	{
		Helpers::SetBit(data.m_tmp_u8, 0, registers);
		memory.Write(registers->HL, data.m_tmp_u8);
		return InstructionResult::Continue;
	}

	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_0_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->A, 0, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_1_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->B, 1, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_1_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->C, 1, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_1_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->D, 1, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_1_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->E, 1, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_1_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->H, 1, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_1_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->L, 1, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_1_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->HL];
		return InstructionResult::Continue;
	}
	else if (data.m_cycles == 1)
	{
		Helpers::SetBit(data.m_tmp_u8, 1, registers);
		memory.Write(registers->HL, data.m_tmp_u8);
		return InstructionResult::Continue;
	}

	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_1_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->A, 1, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_2_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->B, 2, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_2_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->C, 2, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_2_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->D, 2, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_2_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->E, 2, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_2_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->H, 2, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_2_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->L, 2, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_2_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->HL];
		return InstructionResult::Continue;
	}
	else if (data.m_cycles == 1)
	{
		Helpers::SetBit(data.m_tmp_u8, 2, registers);
		memory.Write(registers->HL, data.m_tmp_u8);
		return InstructionResult::Continue;
	}

	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_2_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->A, 2, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_3_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->B, 3, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_3_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->C, 3, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_3_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->D, 3, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_3_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->E, 3, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_3_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->H, 3, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_3_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->L, 3, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_3_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->HL];
		return InstructionResult::Continue;
	}
	else if (data.m_cycles == 1)
	{
		Helpers::SetBit(data.m_tmp_u8, 3, registers);
		memory.Write(registers->HL, data.m_tmp_u8);
		return InstructionResult::Continue;
	}

	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_3_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->A, 3, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_4_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->B, 4, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_4_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->C, 4, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_4_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->D, 4, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_4_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->E, 4, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_4_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->H, 4, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_4_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->L, 4, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_4_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->HL];
		return InstructionResult::Continue;
	}
	else if (data.m_cycles == 1)
	{
		Helpers::SetBit(data.m_tmp_u8, 4, registers);
		memory.Write(registers->HL, data.m_tmp_u8);
		return InstructionResult::Continue;
	}

	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_4_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->A, 4, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_5_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->B, 5, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_5_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->C, 5, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_5_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->D, 5, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_5_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->E, 5, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_5_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->H, 5, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_5_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->L, 5, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_5_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->HL];
		return InstructionResult::Continue;
	}
	else if (data.m_cycles == 1)
	{
		Helpers::SetBit(data.m_tmp_u8, 5, registers);
		memory.Write(registers->HL, data.m_tmp_u8);
		return InstructionResult::Continue;
	}

	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_5_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->A, 5, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_6_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->B, 6, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_6_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->C, 6, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_6_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->D, 6, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_6_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->E, 6, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_6_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->H, 6, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_6_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->L, 6, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_6_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->HL];
		return InstructionResult::Continue;
	}
	else if (data.m_cycles == 1)
	{
		Helpers::SetBit(data.m_tmp_u8, 6, registers);
		memory.Write(registers->HL, data.m_tmp_u8);
		return InstructionResult::Continue;
	}

	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_6_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->A, 6, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_7_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->B, 7, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_7_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->C, 7, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_7_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->D, 7, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_7_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->E, 7, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_7_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->H, 7, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_7_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->L, 7, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_7_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_u8 = memory[registers->HL];
		return InstructionResult::Continue;
	}
	else if (data.m_cycles == 1)
	{
		Helpers::SetBit(data.m_tmp_u8, 7, registers);
		memory.Write(registers->HL, data.m_tmp_u8);
		return InstructionResult::Continue;
	}

	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::SET_7_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	Helpers::SetBit(registers->A, 7, registers);
	
	return InstructionResult::Finished;
}

//16 bit loads 
//-------------------------------------------------------------------------------------------------

InstructionResult InstructionFunctions::LD_BC_nn(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
 	if (data.m_cycles == 0)
	{
		//TODO split this up?
		data.m_tmp_16 = Helpers::Read16Bit(registers->PC, memory);
		return InstructionResult::Delay_2;
	}

	registers->BC = data.m_tmp_16;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_DE_nn(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		//TODO split this up?
		data.m_tmp_16 = Helpers::Read16Bit(registers->PC, memory);
		return InstructionResult::Delay_2;
	}

	registers->DE = data.m_tmp_16;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_HL_nn(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		//TODO split this up?
		data.m_tmp_16 = Helpers::Read16Bit(registers->PC, memory);
		return InstructionResult::Delay_2;
	}

	registers->HL = data.m_tmp_16;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_SP_nn(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		//TODO split this up?
		data.m_tmp_16 = Helpers::Read16Bit(registers->PC, memory);
		return InstructionResult::Delay_2;
	}

	registers->SP = data.m_tmp_16;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_mnn_SP(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		//TODO split this up?
		data.m_tmp_16 = Helpers::Read16Bit(registers->PC, memory);
		return InstructionResult::Delay_2;
	}
	else if (data.m_cycles == 2)
	{
		Helpers::Write16Bit(registers->SP, data.m_tmp_16, memory);
		return InstructionResult::Delay_2;
	}

	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_HL_SP_n(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_s8 = memory[registers->PC++];
		return InstructionResult::Delay_2; //TODO not correct, write to HL needs to be split up
	}

	bool subtraction = data.m_tmp_s8 < 0;
	uint8_t previous = static_cast<uint8_t>(registers->SP);
	registers->HL = registers->SP + data.m_tmp_s8;
	registers->ResetFlag(Registers::Flags::zf);
	registers->ResetFlag(Registers::Flags::n);
	uint8_t operand = static_cast<uint8_t>(data.m_tmp_s8);
	Helpers::SetCarry(previous, operand, registers, false);
	Helpers::SetHalfCarryFlag(previous, operand, false, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::LD_SP_HL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
 	if (data.m_cycles == 0)
	{
		return InstructionResult::Continue;
	}

	registers->SP = registers->HL;  //TODO not correct, write to SP needs to be split up
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::POP_BC(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		//TODO split this up?
		data.m_tmp_16 = Helpers::Read16Bit(registers->SP, memory);
		return InstructionResult::Delay_2;
	}
	registers->BC = data.m_tmp_16;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::POP_DE(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		//TODO split this up?
		data.m_tmp_16 = Helpers::Read16Bit(registers->SP, memory);
		return InstructionResult::Delay_2;
	}
	registers->DE = data.m_tmp_16;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::POP_HL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		//TODO split this up?
		data.m_tmp_16 = Helpers::Read16Bit(registers->SP, memory);
		return InstructionResult::Delay_2;
	}
	registers->HL = data.m_tmp_16;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::POP_AF(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		registers->SetAllFlags(memory[registers->SP++]);
		return InstructionResult::Continue;
	}
	else if (data.m_cycles == 1)
	{
		registers->A = memory[registers->SP++];
		return InstructionResult::Continue;
	}
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::PUSH_BC(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	return Helpers::Push(registers->BC, data.m_cycles, registers, memory);
}

InstructionResult InstructionFunctions::PUSH_DE(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	return Helpers::Push(registers->DE, data.m_cycles, registers, memory);
}

InstructionResult InstructionFunctions::PUSH_HL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	return Helpers::Push(registers->HL, data.m_cycles, registers, memory);
}

InstructionResult InstructionFunctions::PUSH_AF(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	return Helpers::Push(registers->AF, data.m_cycles, registers, memory);
}

//16 bit arithmatic/logic 
//-------------------------------------------------------------------------------------------------
InstructionResult InstructionFunctions::INC_BC(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
 	if (data.m_cycles == 0)
	{
		registers->BC++;
		return InstructionResult::Continue;
	}
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::INC_DE(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		registers->DE++;
		return InstructionResult::Continue;
	}
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::INC_HL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		registers->HL++;
		return InstructionResult::Continue;
	}
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::INC_SP(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		registers->SP++;
		return InstructionResult::Continue;
	}
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::ADD_HL_BC(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		return InstructionResult::Continue;
	}

	Helpers::SetFlags16(registers->HL, registers->BC, registers);
	registers->HL += registers->BC;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::ADD_HL_DE(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		return InstructionResult::Continue;
	}

	Helpers::SetFlags16(registers->HL, registers->DE, registers);
	registers->HL += registers->DE;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::ADD_HL_HL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		return InstructionResult::Continue;
	}

	Helpers::SetFlags16(registers->HL, registers->HL, registers);
	registers->HL += registers->HL;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::ADD_HL_SP(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		return InstructionResult::Continue;
	}

	Helpers::SetFlags16(registers->HL, registers->SP, registers);
	registers->HL += registers->SP;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::ADD_SP_n(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		data.m_tmp_s8 = memory[registers->PC++];
		return InstructionResult::Delay_3;
	}

	bool subtraction = data.m_tmp_s8 < 0;
	uint8_t previous = static_cast<uint8_t>(registers->SP);
	registers->SP += data.m_tmp_s8;
	registers->ResetFlag(Registers::Flags::n);
	registers->ResetFlag(Registers::Flags::zf);
	uint8_t operand = static_cast<uint8_t>(data.m_tmp_s8);

	Helpers::SetCarry(previous, operand, registers, false);
	Helpers::SetHalfCarryFlag(previous, operand, false, registers);
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::DEC_BC(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		return InstructionResult::Continue;
	}

	registers->BC--;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::DEC_DE(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		return InstructionResult::Continue;
	}

	registers->DE--;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::DEC_HL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		return InstructionResult::Continue;
	}

	registers->HL--;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::DEC_SP(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	if (data.m_cycles == 0)
	{
		return InstructionResult::Continue;
	}

	registers->SP--;
	
	return InstructionResult::Finished;
}

InstructionResult InstructionFunctions::INTERRUPT_HANDLING(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory)
{
	switch (data.m_cycles)
	{
	case 0:
		registers->PC--;
		return InstructionResult::Continue;
	case 1:
		return InstructionFunctions::Helpers::Call(0x00, 0, registers, memory);
	case 2:
		data.m_tmp_16 = Interrupts::GetJumpAddrAndClear(memory);
		return InstructionFunctions::Helpers::Call(data.m_tmp_16, 1, registers, memory);
	case 3:
		return InstructionFunctions::Helpers::Call(data.m_tmp_16, 2, registers, memory);
	case 4:
		return InstructionResult::Finished;
	default:
		LOG_ERROR("Invalid execution cycle for INTERRUPT_HANDLING instruction");
		return InstructionResult::Finished;
	}
}
