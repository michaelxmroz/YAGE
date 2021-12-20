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

	void JP_NZ_nn(const char* mnemonic, Registers* registers, uint8_t* memory);
	void JP_Z_nn(const char* mnemonic, Registers* registers, uint8_t* memory);
	void JP_NC_nn(const char* mnemonic, Registers* registers, uint8_t* memory);
	void JP_C_nn(const char* mnemonic, Registers* registers, uint8_t* memory);
	void JP_nn(const char* mnemonic, Registers* registers, uint8_t* memory);
	void JP_HL(const char* mnemonic, Registers* registers, uint8_t* memory);

	void CALL_NZ_nn(const char* mnemonic, Registers* registers, uint8_t* memory);
	void CALL_Z_nn(const char* mnemonic, Registers* registers, uint8_t* memory);
	void CALL_NC_nn(const char* mnemonic, Registers* registers, uint8_t* memory);
	void CALL_C_nn(const char* mnemonic, Registers* registers, uint8_t* memory);
	void CALL_nn(const char* mnemonic, Registers* registers, uint8_t* memory);

	void RST_00(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RST_10(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RST_20(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RST_30(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RST_08(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RST_18(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RST_28(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RST_38(const char* mnemonic, Registers* registers, uint8_t* memory);

	void RET_NZ(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RET_Z(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RET_NC(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RET_C(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RET(const char* mnemonic, Registers* registers, uint8_t* memory);

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

	void LD_C_B(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_C_C(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_C_D(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_C_E(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_C_H(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_C_L(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_C_mHL(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_C_A(const char* mnemonic, Registers* registers, uint8_t* memory);

	void LD_D_B(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_D_C(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_D_D(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_D_E(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_D_H(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_D_L(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_D_mHL(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_D_A(const char* mnemonic, Registers* registers, uint8_t* memory);

	void LD_E_B(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_E_C(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_E_D(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_E_E(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_E_H(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_E_L(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_E_mHL(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_E_A(const char* mnemonic, Registers* registers, uint8_t* memory);

	void LD_H_B(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_H_C(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_H_D(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_H_E(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_H_H(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_H_L(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_H_mHL(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_H_A(const char* mnemonic, Registers* registers, uint8_t* memory);

	void LD_L_B(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_L_C(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_L_D(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_L_E(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_L_H(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_L_L(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_L_mHL(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_L_A(const char* mnemonic, Registers* registers, uint8_t* memory);

	void LD_mHL_B(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_mHL_C(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_mHL_D(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_mHL_E(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_mHL_H(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_mHL_L(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_mHL_A(const char* mnemonic, Registers* registers, uint8_t* memory);

	void LD_A_B(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_A_C(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_A_D(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_A_E(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_A_H(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_A_L(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_A_mHL(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_A_A(const char* mnemonic, Registers* registers, uint8_t* memory);

	void LD_A_mBC(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_A_mDE(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_A_mHLinc(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_A_mHLdec(const char* mnemonic, Registers* registers, uint8_t* memory);

	void LDH_mn_A(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LDH_A_mn(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LDH_mC_A(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LDH_A_mC(const char* mnemonic, Registers* registers, uint8_t* memory);

	void LD_mnn_A(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_A_mnn(const char* mnemonic, Registers* registers, uint8_t* memory);

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

	void ADD_A_B(const char* mnemonic, Registers* registers, uint8_t* memory);
	void ADD_A_C(const char* mnemonic, Registers* registers, uint8_t* memory);
	void ADD_A_D(const char* mnemonic, Registers* registers, uint8_t* memory);
	void ADD_A_E(const char* mnemonic, Registers* registers, uint8_t* memory);
	void ADD_A_H(const char* mnemonic, Registers* registers, uint8_t* memory);
	void ADD_A_L(const char* mnemonic, Registers* registers, uint8_t* memory);
	void ADD_A_mHL(const char* mnemonic, Registers* registers, uint8_t* memory);
	void ADD_A_A(const char* mnemonic, Registers* registers, uint8_t* memory);

	void ADC_A_B(const char* mnemonic, Registers* registers, uint8_t* memory);
	void ADC_A_C(const char* mnemonic, Registers* registers, uint8_t* memory);
	void ADC_A_D(const char* mnemonic, Registers* registers, uint8_t* memory);
	void ADC_A_E(const char* mnemonic, Registers* registers, uint8_t* memory);
	void ADC_A_H(const char* mnemonic, Registers* registers, uint8_t* memory);
	void ADC_A_L(const char* mnemonic, Registers* registers, uint8_t* memory);
	void ADC_A_mHL(const char* mnemonic, Registers* registers, uint8_t* memory);
	void ADC_A_A(const char* mnemonic, Registers* registers, uint8_t* memory);

	void SUB_A_B(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SUB_A_C(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SUB_A_D(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SUB_A_E(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SUB_A_H(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SUB_A_L(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SUB_A_mHL(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SUB_A_A(const char* mnemonic, Registers* registers, uint8_t* memory);

	void SBC_A_B(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SBC_A_C(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SBC_A_D(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SBC_A_E(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SBC_A_H(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SBC_A_L(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SBC_A_mHL(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SBC_A_A(const char* mnemonic, Registers* registers, uint8_t* memory);

	void AND_A_B(const char* mnemonic, Registers* registers, uint8_t* memory);
	void AND_A_C(const char* mnemonic, Registers* registers, uint8_t* memory);
	void AND_A_D(const char* mnemonic, Registers* registers, uint8_t* memory);
	void AND_A_E(const char* mnemonic, Registers* registers, uint8_t* memory);
	void AND_A_H(const char* mnemonic, Registers* registers, uint8_t* memory);
	void AND_A_L(const char* mnemonic, Registers* registers, uint8_t* memory);
	void AND_A_mHL(const char* mnemonic, Registers* registers, uint8_t* memory);
	void AND_A_A(const char* mnemonic, Registers* registers, uint8_t* memory);

	void XOR_A_B(const char* mnemonic, Registers* registers, uint8_t* memory);
	void XOR_A_C(const char* mnemonic, Registers* registers, uint8_t* memory);
	void XOR_A_D(const char* mnemonic, Registers* registers, uint8_t* memory);
	void XOR_A_E(const char* mnemonic, Registers* registers, uint8_t* memory);
	void XOR_A_H(const char* mnemonic, Registers* registers, uint8_t* memory);
	void XOR_A_L(const char* mnemonic, Registers* registers, uint8_t* memory);
	void XOR_A_mHL(const char* mnemonic, Registers* registers, uint8_t* memory);
	void XOR_A_A(const char* mnemonic, Registers* registers, uint8_t* memory);

	void OR_A_B(const char* mnemonic, Registers* registers, uint8_t* memory);
	void OR_A_C(const char* mnemonic, Registers* registers, uint8_t* memory);
	void OR_A_D(const char* mnemonic, Registers* registers, uint8_t* memory);
	void OR_A_E(const char* mnemonic, Registers* registers, uint8_t* memory);
	void OR_A_H(const char* mnemonic, Registers* registers, uint8_t* memory);
	void OR_A_L(const char* mnemonic, Registers* registers, uint8_t* memory);
	void OR_A_mHL(const char* mnemonic, Registers* registers, uint8_t* memory);
	void OR_A_A(const char* mnemonic, Registers* registers, uint8_t* memory);

	void CP_A_B(const char* mnemonic, Registers* registers, uint8_t* memory);
	void CP_A_C(const char* mnemonic, Registers* registers, uint8_t* memory);
	void CP_A_D(const char* mnemonic, Registers* registers, uint8_t* memory);
	void CP_A_E(const char* mnemonic, Registers* registers, uint8_t* memory);
	void CP_A_H(const char* mnemonic, Registers* registers, uint8_t* memory);
	void CP_A_L(const char* mnemonic, Registers* registers, uint8_t* memory);
	void CP_A_mHL(const char* mnemonic, Registers* registers, uint8_t* memory);
	void CP_A_A(const char* mnemonic, Registers* registers, uint8_t* memory);

	void ADD_A_n(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SUB_A_n(const char* mnemonic, Registers* registers, uint8_t* memory);
	void AND_A_n(const char* mnemonic, Registers* registers, uint8_t* memory);
	void OR_A_n(const char* mnemonic, Registers* registers, uint8_t* memory);
	void ADC_A_n(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SBC_A_n(const char* mnemonic, Registers* registers, uint8_t* memory);
	void XOR_A_n(const char* mnemonic, Registers* registers, uint8_t* memory);
	void CP_A_n(const char* mnemonic, Registers* registers, uint8_t* memory);

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

	void LD_HL_SP_n(const char* mnemonic, Registers* registers, uint8_t* memory);
	void LD_SP_HL(const char* mnemonic, Registers* registers, uint8_t* memory);

	void POP_BC(const char* mnemonic, Registers* registers, uint8_t* memory);
	void POP_DE(const char* mnemonic, Registers* registers, uint8_t* memory);
	void POP_HL(const char* mnemonic, Registers* registers, uint8_t* memory);
	void POP_AF(const char* mnemonic, Registers* registers, uint8_t* memory);

	void PUSH_BC(const char* mnemonic, Registers* registers, uint8_t* memory);
	void PUSH_DE(const char* mnemonic, Registers* registers, uint8_t* memory);
	void PUSH_HL(const char* mnemonic, Registers* registers, uint8_t* memory);
	void PUSH_AF(const char* mnemonic, Registers* registers, uint8_t* memory);

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
	void ADD_SP_n(const char* mnemonic, Registers* registers, uint8_t* memory);

	void DEC_BC(const char* mnemonic, Registers* registers, uint8_t* memory);
	void DEC_DE(const char* mnemonic, Registers* registers, uint8_t* memory);
	void DEC_HL(const char* mnemonic, Registers* registers, uint8_t* memory);
	void DEC_SP(const char* mnemonic, Registers* registers, uint8_t* memory);

}

