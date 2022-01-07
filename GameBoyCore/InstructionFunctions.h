#pragma once
#include <cstdint>
#include "Registers.h"
#include "Logging.h"
#include "Memory.h"

namespace InstructionFunctions
{
	namespace Helpers
	{
		FORCE_INLINE void Call(uint16_t addr, Registers* registers, Memory& memory);
	}

	uint32_t UNIMPLEMENTED(const char* mnemonic, Registers* registers, Memory& memory);

	//Control instructions
	//-------------------------------------------------------------------------------------------------
	uint32_t NOP(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SCF(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t CCF(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t JR_NZ_n(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t JR_Z_n(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t JR_NC_n(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t JR_C_n(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t JR_n(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t JP_NZ_nn(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t JP_Z_nn(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t JP_NC_nn(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t JP_C_nn(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t JP_nn(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t JP_HL(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t CALL_NZ_nn(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t CALL_Z_nn(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t CALL_NC_nn(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t CALL_C_nn(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t CALL_nn(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t RST_00(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RST_10(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RST_20(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RST_30(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RST_08(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RST_18(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RST_28(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RST_38(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t RET_NZ(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RET_Z(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RET_NC(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RET_C(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RET(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RETI(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t EI(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t DI(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t HALT(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t STOP(const char* mnemonic, Registers* registers, Memory& memory);

	//8 bit loads 
	//-------------------------------------------------------------------------------------------------
	uint32_t LD_mBC_A(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_mDE_A(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_mHLinc_A(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_mHLdec_A(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t LD_B_n(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_D_n(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_H_n(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_mHL_n(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_C_n(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_E_n(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_L_n(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_A_n(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t LD_B_B(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_B_C(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_B_D(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_B_E(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_B_H(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_B_L(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_B_mHL(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_B_A(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t LD_C_B(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_C_C(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_C_D(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_C_E(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_C_H(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_C_L(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_C_mHL(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_C_A(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t LD_D_B(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_D_C(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_D_D(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_D_E(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_D_H(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_D_L(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_D_mHL(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_D_A(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t LD_E_B(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_E_C(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_E_D(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_E_E(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_E_H(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_E_L(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_E_mHL(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_E_A(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t LD_H_B(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_H_C(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_H_D(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_H_E(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_H_H(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_H_L(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_H_mHL(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_H_A(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t LD_L_B(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_L_C(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_L_D(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_L_E(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_L_H(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_L_L(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_L_mHL(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_L_A(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t LD_mHL_B(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_mHL_C(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_mHL_D(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_mHL_E(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_mHL_H(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_mHL_L(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_mHL_A(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t LD_A_B(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_A_C(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_A_D(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_A_E(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_A_H(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_A_L(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_A_mHL(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_A_A(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t LD_A_mBC(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_A_mDE(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_A_mHLinc(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_A_mHLdec(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t LDH_mn_A(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LDH_A_mn(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LDH_mC_A(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LDH_A_mC(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t LD_mnn_A(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_A_mnn(const char* mnemonic, Registers* registers, Memory& memory);

	//8 bit arithmatic/logic 
	//-------------------------------------------------------------------------------------------------
	uint32_t INC_B(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t INC_D(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t INC_H(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t INC_mHL(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t INC_C(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t INC_E(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t INC_L(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t INC_A(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t DEC_B(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t DEC_D(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t DEC_H(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t DEC_mHL(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t DEC_C(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t DEC_E(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t DEC_L(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t DEC_A(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t CPL(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t ADD_A_B(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t ADD_A_C(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t ADD_A_D(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t ADD_A_E(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t ADD_A_H(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t ADD_A_L(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t ADD_A_mHL(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t ADD_A_A(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t ADC_A_B(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t ADC_A_C(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t ADC_A_D(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t ADC_A_E(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t ADC_A_H(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t ADC_A_L(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t ADC_A_mHL(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t ADC_A_A(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t SUB_A_B(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SUB_A_C(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SUB_A_D(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SUB_A_E(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SUB_A_H(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SUB_A_L(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SUB_A_mHL(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SUB_A_A(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t SBC_A_B(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SBC_A_C(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SBC_A_D(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SBC_A_E(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SBC_A_H(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SBC_A_L(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SBC_A_mHL(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SBC_A_A(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t AND_A_B(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t AND_A_C(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t AND_A_D(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t AND_A_E(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t AND_A_H(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t AND_A_L(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t AND_A_mHL(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t AND_A_A(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t XOR_A_B(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t XOR_A_C(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t XOR_A_D(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t XOR_A_E(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t XOR_A_H(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t XOR_A_L(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t XOR_A_mHL(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t XOR_A_A(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t OR_A_B(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t OR_A_C(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t OR_A_D(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t OR_A_E(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t OR_A_H(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t OR_A_L(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t OR_A_mHL(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t OR_A_A(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t CP_A_B(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t CP_A_C(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t CP_A_D(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t CP_A_E(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t CP_A_H(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t CP_A_L(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t CP_A_mHL(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t CP_A_A(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t ADD_A_n(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SUB_A_n(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t AND_A_n(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t OR_A_n(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t ADC_A_n(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SBC_A_n(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t XOR_A_n(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t CP_A_n(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t DAA(const char* mnemonic, Registers* registers, Memory& memory);

	//8 bit shifts
	//-------------------------------------------------------------------------------------------------
	uint32_t RLCA(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RLA(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RRCA(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RRA(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t RLC_B(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RLC_C(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RLC_D(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RLC_E(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RLC_H(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RLC_L(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RLC_mHL(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RLC_A(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t RRC_B(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RRC_C(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RRC_D(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RRC_E(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RRC_H(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RRC_L(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RRC_mHL(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RRC_A(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t RL_B(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RL_C(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RL_D(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RL_E(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RL_H(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RL_L(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RL_mHL(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RL_A(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t RR_B(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RR_C(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RR_D(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RR_E(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RR_H(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RR_L(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RR_mHL(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RR_A(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t SLA_B(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SLA_C(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SLA_D(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SLA_E(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SLA_H(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SLA_L(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SLA_mHL(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SLA_A(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t SRA_B(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SRA_C(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SRA_D(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SRA_E(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SRA_H(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SRA_L(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SRA_mHL(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SRA_A(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t SWAP_B(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SWAP_C(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SWAP_D(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SWAP_E(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SWAP_H(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SWAP_L(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SWAP_mHL(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SWAP_A(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t SRL_B(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SRL_C(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SRL_D(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SRL_E(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SRL_H(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SRL_L(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SRL_mHL(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SRL_A(const char* mnemonic, Registers* registers, Memory& memory);

	//Singlebit operations
	//-------------------------------------------------------------------------------------------------
	uint32_t BIT_0_B(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t BIT_0_C(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t BIT_0_D(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t BIT_0_E(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t BIT_0_H(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t BIT_0_L(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t BIT_0_mHL(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t BIT_0_A(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t BIT_1_B(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t BIT_1_C(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t BIT_1_D(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t BIT_1_E(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t BIT_1_H(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t BIT_1_L(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t BIT_1_mHL(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t BIT_1_A(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t BIT_2_B(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t BIT_2_C(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t BIT_2_D(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t BIT_2_E(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t BIT_2_H(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t BIT_2_L(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t BIT_2_mHL(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t BIT_2_A(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t BIT_3_B(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t BIT_3_C(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t BIT_3_D(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t BIT_3_E(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t BIT_3_H(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t BIT_3_L(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t BIT_3_mHL(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t BIT_3_A(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t BIT_4_B(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t BIT_4_C(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t BIT_4_D(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t BIT_4_E(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t BIT_4_H(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t BIT_4_L(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t BIT_4_mHL(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t BIT_4_A(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t BIT_5_B(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t BIT_5_C(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t BIT_5_D(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t BIT_5_E(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t BIT_5_H(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t BIT_5_L(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t BIT_5_mHL(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t BIT_5_A(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t BIT_6_B(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t BIT_6_C(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t BIT_6_D(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t BIT_6_E(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t BIT_6_H(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t BIT_6_L(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t BIT_6_mHL(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t BIT_6_A(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t BIT_7_B(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t BIT_7_C(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t BIT_7_D(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t BIT_7_E(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t BIT_7_H(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t BIT_7_L(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t BIT_7_mHL(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t BIT_7_A(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t RES_0_B(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RES_0_C(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RES_0_D(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RES_0_E(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RES_0_H(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RES_0_L(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RES_0_mHL(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RES_0_A(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t RES_1_B(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RES_1_C(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RES_1_D(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RES_1_E(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RES_1_H(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RES_1_L(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RES_1_mHL(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RES_1_A(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t RES_2_B(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RES_2_C(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RES_2_D(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RES_2_E(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RES_2_H(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RES_2_L(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RES_2_mHL(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RES_2_A(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t RES_3_B(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RES_3_C(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RES_3_D(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RES_3_E(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RES_3_H(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RES_3_L(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RES_3_mHL(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RES_3_A(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t RES_4_B(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RES_4_C(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RES_4_D(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RES_4_E(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RES_4_H(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RES_4_L(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RES_4_mHL(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RES_4_A(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t RES_5_B(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RES_5_C(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RES_5_D(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RES_5_E(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RES_5_H(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RES_5_L(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RES_5_mHL(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RES_5_A(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t RES_6_B(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RES_6_C(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RES_6_D(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RES_6_E(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RES_6_H(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RES_6_L(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RES_6_mHL(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RES_6_A(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t RES_7_B(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RES_7_C(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RES_7_D(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RES_7_E(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RES_7_H(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RES_7_L(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RES_7_mHL(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t RES_7_A(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t SET_0_B(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SET_0_C(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SET_0_D(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SET_0_E(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SET_0_H(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SET_0_L(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SET_0_mHL(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SET_0_A(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t SET_1_B(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SET_1_C(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SET_1_D(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SET_1_E(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SET_1_H(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SET_1_L(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SET_1_mHL(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SET_1_A(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t SET_2_B(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SET_2_C(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SET_2_D(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SET_2_E(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SET_2_H(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SET_2_L(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SET_2_mHL(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SET_2_A(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t SET_3_B(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SET_3_C(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SET_3_D(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SET_3_E(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SET_3_H(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SET_3_L(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SET_3_mHL(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SET_3_A(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t SET_4_B(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SET_4_C(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SET_4_D(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SET_4_E(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SET_4_H(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SET_4_L(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SET_4_mHL(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SET_4_A(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t SET_5_B(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SET_5_C(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SET_5_D(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SET_5_E(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SET_5_H(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SET_5_L(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SET_5_mHL(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SET_5_A(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t SET_6_B(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SET_6_C(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SET_6_D(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SET_6_E(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SET_6_H(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SET_6_L(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SET_6_mHL(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SET_6_A(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t SET_7_B(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SET_7_C(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SET_7_D(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SET_7_E(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SET_7_H(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SET_7_L(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SET_7_mHL(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t SET_7_A(const char* mnemonic, Registers* registers, Memory& memory);

	//16 bit loads 
	//-------------------------------------------------------------------------------------------------
	uint32_t LD_BC_nn(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_DE_nn(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_HL_nn(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_SP_nn(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t LD_mnn_SP(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t LD_HL_SP_n(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t LD_SP_HL(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t POP_BC(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t POP_DE(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t POP_HL(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t POP_AF(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t PUSH_BC(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t PUSH_DE(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t PUSH_HL(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t PUSH_AF(const char* mnemonic, Registers* registers, Memory& memory);

	//16 bit arithmatic/logic 
	//-------------------------------------------------------------------------------------------------
	uint32_t INC_BC(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t INC_DE(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t INC_HL(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t INC_SP(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t ADD_HL_BC(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t ADD_HL_DE(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t ADD_HL_HL(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t ADD_HL_SP(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t ADD_SP_n(const char* mnemonic, Registers* registers, Memory& memory);

	uint32_t DEC_BC(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t DEC_DE(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t DEC_HL(const char* mnemonic, Registers* registers, Memory& memory);
	uint32_t DEC_SP(const char* mnemonic, Registers* registers, Memory& memory);

}

