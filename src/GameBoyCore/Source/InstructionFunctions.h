#pragma once
#include <cstdint>
#include "Registers.h"
#include "Logging.h"
#include "Memory.h"

enum class InstructionResult : uint8_t
{
	Finished = 0,
	Continue = 1,
	Delay_2 = 2,
	Delay_3 = 3,
};

struct InstructionTempData
{
	void Reset()
	{
		m_tmp_16 = 0;
		m_tmp_s8 = 0;
		m_tmp_u8 = 0;
		m_cycles = 0;
		m_delay = 0;
	}
	uint16_t m_tmp_16;
	int8_t m_tmp_s8;
	uint8_t m_tmp_u8;
	uint8_t m_cycles;
	uint8_t m_delay;
};

namespace InstructionFunctions
{

	InstructionResult UNIMPLEMENTED(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	//Control instructions
	//-------------------------------------------------------------------------------------------------
	InstructionResult NOP(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SCF(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult CCF(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult JR_NZ_n(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult JR_Z_n(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult JR_NC_n(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult JR_C_n(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult JR_n(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult JP_NZ_nn(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult JP_Z_nn(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult JP_NC_nn(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult JP_C_nn(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult JP_nn(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult JP_HL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult CALL_NZ_nn(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult CALL_Z_nn(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult CALL_NC_nn(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult CALL_C_nn(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult CALL_nn(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult RST_00(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RST_10(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RST_20(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RST_30(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RST_08(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RST_18(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RST_28(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RST_38(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult RET_NZ(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RET_Z(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RET_NC(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RET_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RET(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RETI(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult EI(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult DI(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult HALT(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult STOP(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	//8 bit loads 
	//-------------------------------------------------------------------------------------------------
	InstructionResult LD_mBC_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_mDE_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_mHLinc_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_mHLdec_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult LD_B_n(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_D_n(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_H_n(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_mHL_n(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_C_n(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_E_n(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_L_n(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_A_n(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult LD_B_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_B_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_B_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_B_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_B_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_B_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_B_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_B_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult LD_C_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_C_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_C_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_C_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_C_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_C_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_C_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_C_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult LD_D_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_D_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_D_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_D_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_D_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_D_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_D_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_D_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult LD_E_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_E_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_E_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_E_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_E_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_E_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_E_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_E_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult LD_H_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_H_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_H_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_H_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_H_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_H_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_H_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_H_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult LD_L_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_L_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_L_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_L_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_L_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_L_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_L_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_L_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult LD_mHL_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_mHL_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_mHL_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_mHL_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_mHL_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_mHL_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_mHL_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult LD_A_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_A_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_A_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_A_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_A_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_A_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_A_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_A_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult LD_A_mBC(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_A_mDE(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_A_mHLinc(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_A_mHLdec(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult LDH_mn_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LDH_A_mn(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LDH_mC_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LDH_A_mC(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult LD_mnn_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_A_mnn(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	//8 bit arithmatic/logic 
	//-------------------------------------------------------------------------------------------------
	InstructionResult INC_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult INC_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult INC_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult INC_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult INC_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult INC_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult INC_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult INC_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult DEC_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult DEC_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult DEC_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult DEC_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult DEC_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult DEC_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult DEC_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult DEC_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult CPL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult ADD_A_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult ADD_A_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult ADD_A_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult ADD_A_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult ADD_A_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult ADD_A_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult ADD_A_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult ADD_A_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult ADC_A_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult ADC_A_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult ADC_A_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult ADC_A_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult ADC_A_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult ADC_A_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult ADC_A_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult ADC_A_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult SUB_A_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SUB_A_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SUB_A_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SUB_A_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SUB_A_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SUB_A_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SUB_A_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SUB_A_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult SBC_A_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SBC_A_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SBC_A_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SBC_A_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SBC_A_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SBC_A_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SBC_A_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SBC_A_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult AND_A_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult AND_A_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult AND_A_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult AND_A_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult AND_A_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult AND_A_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult AND_A_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult AND_A_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult XOR_A_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult XOR_A_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult XOR_A_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult XOR_A_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult XOR_A_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult XOR_A_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult XOR_A_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult XOR_A_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult OR_A_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult OR_A_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult OR_A_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult OR_A_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult OR_A_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult OR_A_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult OR_A_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult OR_A_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult CP_A_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult CP_A_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult CP_A_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult CP_A_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult CP_A_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult CP_A_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult CP_A_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult CP_A_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult ADD_A_n(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SUB_A_n(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult AND_A_n(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult OR_A_n(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult ADC_A_n(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SBC_A_n(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult XOR_A_n(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult CP_A_n(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult DAA(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	//8 bit shifts
	//-------------------------------------------------------------------------------------------------
	InstructionResult RLCA(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RLA(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RRCA(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RRA(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult RLC_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RLC_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RLC_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RLC_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RLC_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RLC_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RLC_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RLC_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult RRC_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RRC_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RRC_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RRC_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RRC_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RRC_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RRC_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RRC_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult RL_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RL_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RL_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RL_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RL_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RL_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RL_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RL_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult RR_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RR_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RR_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RR_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RR_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RR_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RR_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RR_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult SLA_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SLA_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SLA_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SLA_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SLA_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SLA_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SLA_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SLA_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult SRA_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SRA_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SRA_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SRA_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SRA_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SRA_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SRA_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SRA_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult SWAP_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SWAP_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SWAP_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SWAP_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SWAP_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SWAP_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SWAP_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SWAP_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult SRL_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SRL_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SRL_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SRL_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SRL_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SRL_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SRL_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SRL_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	//Singlebit operations
	//-------------------------------------------------------------------------------------------------
	InstructionResult BIT_0_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult BIT_0_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult BIT_0_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult BIT_0_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult BIT_0_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult BIT_0_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult BIT_0_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult BIT_0_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult BIT_1_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult BIT_1_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult BIT_1_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult BIT_1_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult BIT_1_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult BIT_1_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult BIT_1_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult BIT_1_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult BIT_2_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult BIT_2_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult BIT_2_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult BIT_2_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult BIT_2_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult BIT_2_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult BIT_2_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult BIT_2_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult BIT_3_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult BIT_3_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult BIT_3_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult BIT_3_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult BIT_3_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult BIT_3_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult BIT_3_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult BIT_3_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult BIT_4_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult BIT_4_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult BIT_4_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult BIT_4_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult BIT_4_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult BIT_4_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult BIT_4_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult BIT_4_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult BIT_5_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult BIT_5_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult BIT_5_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult BIT_5_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult BIT_5_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult BIT_5_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult BIT_5_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult BIT_5_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult BIT_6_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult BIT_6_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult BIT_6_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult BIT_6_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult BIT_6_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult BIT_6_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult BIT_6_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult BIT_6_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult BIT_7_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult BIT_7_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult BIT_7_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult BIT_7_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult BIT_7_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult BIT_7_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult BIT_7_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult BIT_7_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult RES_0_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RES_0_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RES_0_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RES_0_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RES_0_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RES_0_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RES_0_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RES_0_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult RES_1_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RES_1_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RES_1_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RES_1_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RES_1_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RES_1_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RES_1_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RES_1_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult RES_2_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RES_2_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RES_2_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RES_2_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RES_2_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RES_2_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RES_2_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RES_2_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult RES_3_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RES_3_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RES_3_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RES_3_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RES_3_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RES_3_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RES_3_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RES_3_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult RES_4_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RES_4_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RES_4_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RES_4_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RES_4_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RES_4_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RES_4_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RES_4_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult RES_5_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RES_5_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RES_5_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RES_5_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RES_5_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RES_5_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RES_5_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RES_5_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult RES_6_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RES_6_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RES_6_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RES_6_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RES_6_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RES_6_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RES_6_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RES_6_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult RES_7_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RES_7_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RES_7_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RES_7_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RES_7_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RES_7_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RES_7_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult RES_7_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult SET_0_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SET_0_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SET_0_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SET_0_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SET_0_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SET_0_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SET_0_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SET_0_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult SET_1_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SET_1_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SET_1_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SET_1_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SET_1_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SET_1_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SET_1_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SET_1_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult SET_2_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SET_2_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SET_2_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SET_2_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SET_2_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SET_2_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SET_2_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SET_2_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult SET_3_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SET_3_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SET_3_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SET_3_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SET_3_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SET_3_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SET_3_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SET_3_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult SET_4_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SET_4_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SET_4_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SET_4_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SET_4_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SET_4_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SET_4_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SET_4_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult SET_5_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SET_5_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SET_5_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SET_5_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SET_5_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SET_5_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SET_5_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SET_5_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult SET_6_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SET_6_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SET_6_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SET_6_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SET_6_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SET_6_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SET_6_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SET_6_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult SET_7_B(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SET_7_C(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SET_7_D(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SET_7_E(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SET_7_H(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SET_7_L(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SET_7_mHL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult SET_7_A(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	//16 bit loads 
	//-------------------------------------------------------------------------------------------------
	InstructionResult LD_BC_nn(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_DE_nn(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_HL_nn(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_SP_nn(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult LD_mnn_SP(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult LD_HL_SP_n(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult LD_SP_HL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult POP_BC(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult POP_DE(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult POP_HL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult POP_AF(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult PUSH_BC(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult PUSH_DE(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult PUSH_HL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult PUSH_AF(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	//16 bit arithmatic/logic 
	//-------------------------------------------------------------------------------------------------
	InstructionResult INC_BC(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult INC_DE(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult INC_HL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult INC_SP(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult ADD_HL_BC(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult ADD_HL_DE(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult ADD_HL_HL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult ADD_HL_SP(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult ADD_SP_n(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult DEC_BC(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult DEC_DE(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult DEC_HL(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
	InstructionResult DEC_SP(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	InstructionResult INTERRUPT_HANDLING(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);
}

