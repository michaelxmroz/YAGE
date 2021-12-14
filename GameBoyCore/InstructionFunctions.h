#pragma once
#include <cstdint>
#include "Registers.h"
#include "Logging.h"

namespace InstructionFunctions
{
	void UNIMPLEMENTED(const char* mnemonic, Registers* registers, uint8_t* memory);

	//Control instructions
	//-------------------------------------------------------------------------------------------------
	void NOP(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SCF(const char* mnemonic, Registers* registers, uint8_t* memory);
	void CCF(const char* mnemonic, Registers* registers, uint8_t* memory);
	void JR_NZ_n(const char* mnemonic, Registers* registers, uint8_t* memory);
	void JR_Z_n(const char* mnemonic, Registers* registers, uint8_t* memory);
	void JR_NC_n(const char* mnemonic, Registers* registers, uint8_t* memory);
	void JR_C_n(const char* mnemonic, Registers* registers, uint8_t* memory);
	void JR_n(const char* mnemonic, Registers* registers, uint8_t* memory);
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
	void LD_C_n(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_E_n(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_L_n(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_A_n(const char* mnemonic, Registers* registers, uint8_t* memory);

	void LD_B_B(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_B_C(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_B_D(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_B_E(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_B_H(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_B_L(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_B_mHL(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_B_A(const char* mnemonic, Registers* registers, uint8_t* memory);

	void LD_A_mBC(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_A_mDE(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_A_mHLinc(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_A_mHLdec(const char* mnemonic, Registers* registers, uint8_t* memory);

	//8 bit arithmatic/logic 
	//-------------------------------------------------------------------------------------------------
	void INC_B(const char* mnemonic, Registers* registers, uint8_t* memory);
	void INC_D(const char* mnemonic, Registers* registers, uint8_t* memory);
	void INC_H(const char* mnemonic, Registers* registers, uint8_t* memory);
	void INC_mHL(const char* mnemonic, Registers* registers, uint8_t* memory);
	void INC_C(const char* mnemonic, Registers* registers, uint8_t* memory);
	void INC_E(const char* mnemonic, Registers* registers, uint8_t* memory);
	void INC_L(const char* mnemonic, Registers* registers, uint8_t* memory);
	void INC_A(const char* mnemonic, Registers* registers, uint8_t* memory);

	void DEC_B(const char* mnemonic, Registers* registers, uint8_t* memory);
	void DEC_D(const char* mnemonic, Registers* registers, uint8_t* memory);
	void DEC_H(const char* mnemonic, Registers* registers, uint8_t* memory);
	void DEC_mHL(const char* mnemonic, Registers* registers, uint8_t* memory);
	void DEC_C(const char* mnemonic, Registers* registers, uint8_t* memory);
	void DEC_E(const char* mnemonic, Registers* registers, uint8_t* memory);
	void DEC_L(const char* mnemonic, Registers* registers, uint8_t* memory);
	void DEC_A(const char* mnemonic, Registers* registers, uint8_t* memory);

	void CPL(const char* mnemonic, Registers* registers, uint8_t* memory);

	//8 bit shifts
	//-------------------------------------------------------------------------------------------------
	void RLCA(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RLA(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RRCA(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RRA(const char* mnemonic, Registers* registers, uint8_t* memory);

	//16 bit loads 
	//-------------------------------------------------------------------------------------------------
	void LD_BC_nn(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_DE_nn(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_HL_nn(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_SP_nn(const char* mnemonic, Registers* registers, uint8_t* memory);

	void LD_mnn_SP(const char* mnemonic, Registers* registers, uint8_t* memory);

	//16 bit arithmatic/logic 
	//-------------------------------------------------------------------------------------------------
	void INC_BC(const char* mnemonic, Registers* registers, uint8_t* memory);
	void INC_DE(const char* mnemonic, Registers* registers, uint8_t* memory);
	void INC_HL(const char* mnemonic, Registers* registers, uint8_t* memory);
	void INC_SP(const char* mnemonic, Registers* registers, uint8_t* memory);

	void ADD_HL_BC(const char* mnemonic, Registers* registers, uint8_t* memory);
	void ADD_HL_DE(const char* mnemonic, Registers* registers, uint8_t* memory);
	void ADD_HL_HL(const char* mnemonic, Registers* registers, uint8_t* memory);
	void ADD_HL_SP(const char* mnemonic, Registers* registers, uint8_t* memory);

	void DEC_BC(const char* mnemonic, Registers* registers, uint8_t* memory);
	void DEC_DE(const char* mnemonic, Registers* registers, uint8_t* memory);
	void DEC_HL(const char* mnemonic, Registers* registers, uint8_t* memory);
	void DEC_SP(const char* mnemonic, Registers* registers, uint8_t* memory);

}

