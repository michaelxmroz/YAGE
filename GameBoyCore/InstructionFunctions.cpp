#include "InstructionFunctions.h"

#define FORCE_INLINE __inline

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
	}
}



void InstructionFunctions::LoadToRegister(const char* mnemonic, Registers* registers, uint8_t* memory)
{
	LOG_INSTRUCTION(mnemonic);
}

void InstructionFunctions::Nop(const char* mnemonic, Registers* registers, uint8_t* memory)
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
