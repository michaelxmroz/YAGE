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
	void RETI(const char* mnemonic, Registers* registers, uint8_t* memory);

	void EI(const char* mnemonic, Registers* registers, uint8_t* memory);
	void DI(const char* mnemonic, Registers* registers, uint8_t* memory);

	void HALT(const char* mnemonic, Registers* registers, uint8_t* memory);
	void STOP(const char* mnemonic, Registers* registers, uint8_t* memory);

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

	void DAA(const char* mnemonic, Registers* registers, uint8_t* memory);

	//8 bit shifts
	//-------------------------------------------------------------------------------------------------
	void RLCA(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RLA(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RRCA(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RRA(const char* mnemonic, Registers* registers, uint8_t* memory);

	void RLC_B(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RLC_C(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RLC_D(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RLC_E(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RLC_H(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RLC_L(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RLC_mHL(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RLC_A(const char* mnemonic, Registers* registers, uint8_t* memory);

	void RRC_B(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RRC_C(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RRC_D(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RRC_E(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RRC_H(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RRC_L(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RRC_mHL(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RRC_A(const char* mnemonic, Registers* registers, uint8_t* memory);

	void RL_B(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RL_C(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RL_D(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RL_E(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RL_H(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RL_L(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RL_mHL(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RL_A(const char* mnemonic, Registers* registers, uint8_t* memory);

	void RR_B(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RR_C(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RR_D(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RR_E(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RR_H(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RR_L(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RR_mHL(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RR_A(const char* mnemonic, Registers* registers, uint8_t* memory);

	void SLA_B(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SLA_C(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SLA_D(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SLA_E(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SLA_H(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SLA_L(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SLA_mHL(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SLA_A(const char* mnemonic, Registers* registers, uint8_t* memory);

	void SRA_B(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SRA_C(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SRA_D(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SRA_E(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SRA_H(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SRA_L(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SRA_mHL(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SRA_A(const char* mnemonic, Registers* registers, uint8_t* memory);

	void SWAP_B(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SWAP_C(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SWAP_D(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SWAP_E(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SWAP_H(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SWAP_L(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SWAP_mHL(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SWAP_A(const char* mnemonic, Registers* registers, uint8_t* memory);

	void SRL_B(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SRL_C(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SRL_D(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SRL_E(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SRL_H(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SRL_L(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SRL_mHL(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SRL_A(const char* mnemonic, Registers* registers, uint8_t* memory);

	//Singlebit operations
	//-------------------------------------------------------------------------------------------------
	void BIT_0_B(const char* mnemonic, Registers* registers, uint8_t* memory);
	void BIT_0_C(const char* mnemonic, Registers* registers, uint8_t* memory);
	void BIT_0_D(const char* mnemonic, Registers* registers, uint8_t* memory);
	void BIT_0_E(const char* mnemonic, Registers* registers, uint8_t* memory);
	void BIT_0_H(const char* mnemonic, Registers* registers, uint8_t* memory);
	void BIT_0_L(const char* mnemonic, Registers* registers, uint8_t* memory);
	void BIT_0_mHL(const char* mnemonic, Registers* registers, uint8_t* memory);
	void BIT_0_A(const char* mnemonic, Registers* registers, uint8_t* memory);

	void BIT_1_B(const char* mnemonic, Registers* registers, uint8_t* memory);
	void BIT_1_C(const char* mnemonic, Registers* registers, uint8_t* memory);
	void BIT_1_D(const char* mnemonic, Registers* registers, uint8_t* memory);
	void BIT_1_E(const char* mnemonic, Registers* registers, uint8_t* memory);
	void BIT_1_H(const char* mnemonic, Registers* registers, uint8_t* memory);
	void BIT_1_L(const char* mnemonic, Registers* registers, uint8_t* memory);
	void BIT_1_mHL(const char* mnemonic, Registers* registers, uint8_t* memory);
	void BIT_1_A(const char* mnemonic, Registers* registers, uint8_t* memory);

	void BIT_2_B(const char* mnemonic, Registers* registers, uint8_t* memory);
	void BIT_2_C(const char* mnemonic, Registers* registers, uint8_t* memory);
	void BIT_2_D(const char* mnemonic, Registers* registers, uint8_t* memory);
	void BIT_2_E(const char* mnemonic, Registers* registers, uint8_t* memory);
	void BIT_2_H(const char* mnemonic, Registers* registers, uint8_t* memory);
	void BIT_2_L(const char* mnemonic, Registers* registers, uint8_t* memory);
	void BIT_2_mHL(const char* mnemonic, Registers* registers, uint8_t* memory);
	void BIT_2_A(const char* mnemonic, Registers* registers, uint8_t* memory);

	void BIT_3_B(const char* mnemonic, Registers* registers, uint8_t* memory);
	void BIT_3_C(const char* mnemonic, Registers* registers, uint8_t* memory);
	void BIT_3_D(const char* mnemonic, Registers* registers, uint8_t* memory);
	void BIT_3_E(const char* mnemonic, Registers* registers, uint8_t* memory);
	void BIT_3_H(const char* mnemonic, Registers* registers, uint8_t* memory);
	void BIT_3_L(const char* mnemonic, Registers* registers, uint8_t* memory);
	void BIT_3_mHL(const char* mnemonic, Registers* registers, uint8_t* memory);
	void BIT_3_A(const char* mnemonic, Registers* registers, uint8_t* memory);

	void BIT_4_B(const char* mnemonic, Registers* registers, uint8_t* memory);
	void BIT_4_C(const char* mnemonic, Registers* registers, uint8_t* memory);
	void BIT_4_D(const char* mnemonic, Registers* registers, uint8_t* memory);
	void BIT_4_E(const char* mnemonic, Registers* registers, uint8_t* memory);
	void BIT_4_H(const char* mnemonic, Registers* registers, uint8_t* memory);
	void BIT_4_L(const char* mnemonic, Registers* registers, uint8_t* memory);
	void BIT_4_mHL(const char* mnemonic, Registers* registers, uint8_t* memory);
	void BIT_4_A(const char* mnemonic, Registers* registers, uint8_t* memory);

	void BIT_5_B(const char* mnemonic, Registers* registers, uint8_t* memory);
	void BIT_5_C(const char* mnemonic, Registers* registers, uint8_t* memory);
	void BIT_5_D(const char* mnemonic, Registers* registers, uint8_t* memory);
	void BIT_5_E(const char* mnemonic, Registers* registers, uint8_t* memory);
	void BIT_5_H(const char* mnemonic, Registers* registers, uint8_t* memory);
	void BIT_5_L(const char* mnemonic, Registers* registers, uint8_t* memory);
	void BIT_5_mHL(const char* mnemonic, Registers* registers, uint8_t* memory);
	void BIT_5_A(const char* mnemonic, Registers* registers, uint8_t* memory);

	void BIT_6_B(const char* mnemonic, Registers* registers, uint8_t* memory);
	void BIT_6_C(const char* mnemonic, Registers* registers, uint8_t* memory);
	void BIT_6_D(const char* mnemonic, Registers* registers, uint8_t* memory);
	void BIT_6_E(const char* mnemonic, Registers* registers, uint8_t* memory);
	void BIT_6_H(const char* mnemonic, Registers* registers, uint8_t* memory);
	void BIT_6_L(const char* mnemonic, Registers* registers, uint8_t* memory);
	void BIT_6_mHL(const char* mnemonic, Registers* registers, uint8_t* memory);
	void BIT_6_A(const char* mnemonic, Registers* registers, uint8_t* memory);

	void BIT_7_B(const char* mnemonic, Registers* registers, uint8_t* memory);
	void BIT_7_C(const char* mnemonic, Registers* registers, uint8_t* memory);
	void BIT_7_D(const char* mnemonic, Registers* registers, uint8_t* memory);
	void BIT_7_E(const char* mnemonic, Registers* registers, uint8_t* memory);
	void BIT_7_H(const char* mnemonic, Registers* registers, uint8_t* memory);
	void BIT_7_L(const char* mnemonic, Registers* registers, uint8_t* memory);
	void BIT_7_mHL(const char* mnemonic, Registers* registers, uint8_t* memory);
	void BIT_7_A(const char* mnemonic, Registers* registers, uint8_t* memory);

	void RES_0_B(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RES_0_C(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RES_0_D(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RES_0_E(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RES_0_H(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RES_0_L(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RES_0_mHL(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RES_0_A(const char* mnemonic, Registers* registers, uint8_t* memory);

	void RES_1_B(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RES_1_C(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RES_1_D(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RES_1_E(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RES_1_H(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RES_1_L(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RES_1_mHL(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RES_1_A(const char* mnemonic, Registers* registers, uint8_t* memory);

	void RES_2_B(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RES_2_C(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RES_2_D(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RES_2_E(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RES_2_H(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RES_2_L(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RES_2_mHL(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RES_2_A(const char* mnemonic, Registers* registers, uint8_t* memory);

	void RES_3_B(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RES_3_C(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RES_3_D(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RES_3_E(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RES_3_H(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RES_3_L(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RES_3_mHL(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RES_3_A(const char* mnemonic, Registers* registers, uint8_t* memory);

	void RES_4_B(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RES_4_C(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RES_4_D(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RES_4_E(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RES_4_H(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RES_4_L(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RES_4_mHL(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RES_4_A(const char* mnemonic, Registers* registers, uint8_t* memory);

	void RES_5_B(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RES_5_C(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RES_5_D(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RES_5_E(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RES_5_H(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RES_5_L(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RES_5_mHL(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RES_5_A(const char* mnemonic, Registers* registers, uint8_t* memory);

	void RES_6_B(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RES_6_C(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RES_6_D(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RES_6_E(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RES_6_H(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RES_6_L(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RES_6_mHL(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RES_6_A(const char* mnemonic, Registers* registers, uint8_t* memory);

	void RES_7_B(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RES_7_C(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RES_7_D(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RES_7_E(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RES_7_H(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RES_7_L(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RES_7_mHL(const char* mnemonic, Registers* registers, uint8_t* memory);
	void RES_7_A(const char* mnemonic, Registers* registers, uint8_t* memory);

	void SET_0_B(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SET_0_C(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SET_0_D(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SET_0_E(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SET_0_H(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SET_0_L(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SET_0_mHL(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SET_0_A(const char* mnemonic, Registers* registers, uint8_t* memory);

	void SET_1_B(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SET_1_C(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SET_1_D(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SET_1_E(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SET_1_H(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SET_1_L(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SET_1_mHL(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SET_1_A(const char* mnemonic, Registers* registers, uint8_t* memory);

	void SET_2_B(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SET_2_C(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SET_2_D(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SET_2_E(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SET_2_H(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SET_2_L(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SET_2_mHL(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SET_2_A(const char* mnemonic, Registers* registers, uint8_t* memory);

	void SET_3_B(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SET_3_C(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SET_3_D(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SET_3_E(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SET_3_H(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SET_3_L(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SET_3_mHL(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SET_3_A(const char* mnemonic, Registers* registers, uint8_t* memory);

	void SET_4_B(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SET_4_C(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SET_4_D(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SET_4_E(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SET_4_H(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SET_4_L(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SET_4_mHL(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SET_4_A(const char* mnemonic, Registers* registers, uint8_t* memory);

	void SET_5_B(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SET_5_C(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SET_5_D(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SET_5_E(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SET_5_H(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SET_5_L(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SET_5_mHL(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SET_5_A(const char* mnemonic, Registers* registers, uint8_t* memory);

	void SET_6_B(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SET_6_C(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SET_6_D(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SET_6_E(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SET_6_H(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SET_6_L(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SET_6_mHL(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SET_6_A(const char* mnemonic, Registers* registers, uint8_t* memory);

	void SET_7_B(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SET_7_C(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SET_7_D(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SET_7_E(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SET_7_H(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SET_7_L(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SET_7_mHL(const char* mnemonic, Registers* registers, uint8_t* memory);
	void SET_7_A(const char* mnemonic, Registers* registers, uint8_t* memory);

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

