#pragma once
#include <cstdint>
#include "Registers.h"
#include "Logging.h"

namespace InstructionFunctions
{
	void LoadToRegister(const char* mnemonic, Registers* registers, uint8_t* memory);
	void NOP(const char* mnemonic, Registers* registers, uint8_t* memory);

	//8 bit loads 
	//-------------------------------------------------------------------------------------------------
	void LD_mBC_A(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_mDE_A(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_mHLinc_A(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_mHLdec_A(const char* mnemonic, Registers* registers, uint8_t* memory);

	void LD_B_n(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_D_n(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_H_n(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_mHL_n(const char* mnemonic, Registers* registers, uint8_t* memory);

	//8 bit arithmatic/logic 
	//-------------------------------------------------------------------------------------------------
	void INC_B(const char* mnemonic, Registers* registers, uint8_t* memory);
	void INC_D(const char* mnemonic, Registers* registers, uint8_t* memory);
	void INC_H(const char* mnemonic, Registers* registers, uint8_t* memory);
	void INC_mHL(const char* mnemonic, Registers* registers, uint8_t* memory);

	void DEC_B(const char* mnemonic, Registers* registers, uint8_t* memory);
	void DEC_D(const char* mnemonic, Registers* registers, uint8_t* memory);
	void DEC_H(const char* mnemonic, Registers* registers, uint8_t* memory);
	void DEC_mHL(const char* mnemonic, Registers* registers, uint8_t* memory);

	//8 bit shifts
	//-------------------------------------------------------------------------------------------------
	void RLCA(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RLA(const char* mnemonic, Registers* registers, uint8_t* memory);

	//16 bit loads 
	//-------------------------------------------------------------------------------------------------
	void LD_BC_nn(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_DE_nn(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_HL_nn(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_SP_nn(const char* mnemonic, Registers* registers, uint8_t* memory);

	//16 bit arithmatic/logic 
	//-------------------------------------------------------------------------------------------------
	void INC_BC(const char* mnemonic, Registers* registers, uint8_t* memory);
	void INC_DE(const char* mnemonic, Registers* registers, uint8_t* memory);
	void INC_HL(const char* mnemonic, Registers* registers, uint8_t* memory);
	void INC_SP(const char* mnemonic, Registers* registers, uint8_t* memory);
}

