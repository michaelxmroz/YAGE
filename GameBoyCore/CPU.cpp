#include "CPU.h"

CPU::CPU()  
	: m_registers()
	, m_instructions
{
	  { "NOP", 1, 1, &InstructionFunctions::NOP }
	, { "LD BC nn", 3, 3, &InstructionFunctions::LD_BC_nn }
	, { "LD (BC) A", 1, 2, &InstructionFunctions::LD_mBC_A }
	, { "INC BC", 1, 2, &InstructionFunctions::INC_BC }
	, { "INC B", 1, 1, &InstructionFunctions::INC_B }
	, { "DEC B", 1, 1, &InstructionFunctions::DEC_B }
	, { "LD B n", 2, 2, &InstructionFunctions::LD_B_n }
	, { "RLCA", 1, 1, &InstructionFunctions::RLCA }
	, { "LD (nn) SP", 3, 5, &InstructionFunctions::LD_mnn_SP }
	, { "ADD HL BC", 1, 2, &InstructionFunctions::ADD_HL_BC }
	, { "LD A (BC)", 1, 2, &InstructionFunctions::LD_A_mBC }
	, { "DEC BC", 1, 2, &InstructionFunctions::DEC_BC }
	, { "INC C", 1, 1, &InstructionFunctions::INC_C }
	, { "DEC C", 1, 1, &InstructionFunctions::DEC_C }
	, { "LD C n", 2, 2, &InstructionFunctions::LD_C_n }
	, { "RRCA", 1, 1, &InstructionFunctions::RRCA }
	, { "STOP", 1, 2, &InstructionFunctions::STOP }
	, { "LD DE nn", 3, 3, &InstructionFunctions::LD_DE_nn }
	, { "LD (DE) A", 1, 2, &InstructionFunctions::LD_mDE_A }
	, { "INC DE", 1, 2, &InstructionFunctions::INC_DE }
	, { "INC D", 1, 1, &InstructionFunctions::INC_D }
	, { "DEC D", 1, 1, &InstructionFunctions::DEC_D }
	, { "LD D n", 2, 2, &InstructionFunctions::LD_D_n }
	, { "RLA", 1, 1, &InstructionFunctions::RLA }
	, { "JR n", 2, 2, &InstructionFunctions::JR_n }
	, { "ADD HL DE", 1, 2, &InstructionFunctions::ADD_HL_DE }
	, { "LD A (DE)", 1, 2, &InstructionFunctions::LD_A_mDE }
	, { "DEC DE", 1, 2, &InstructionFunctions::DEC_DE }
	, { "INC E", 1, 1, &InstructionFunctions::INC_E }
	, { "DEC E", 1, 1, &InstructionFunctions::DEC_E }
	, { "LD E n", 2, 2, &InstructionFunctions::LD_E_n }
	, { "RRA", 1, 1, &InstructionFunctions::RRA }
	, { "JR NZ n", 2, 2, &InstructionFunctions::JR_NZ_n }
	, { "LD HL nn", 3, 3, &InstructionFunctions::LD_HL_nn }
	, { "LD (HL+) A", 1, 2, &InstructionFunctions::LD_mHLinc_A }
	, { "INC HL", 1, 1, &InstructionFunctions::INC_HL }
	, { "INC H", 1, 1, &InstructionFunctions::INC_H }
	, { "DEC H", 1, 1, &InstructionFunctions::DEC_H }
	, { "LD H n", 2, 2, &InstructionFunctions::LD_H_n }
	, { "DAA", 1, 1, &InstructionFunctions::DAA }
	, { "JR Z n", 2, 2, &InstructionFunctions::JR_Z_n }
	, { "ADD HL HL", 1, 2, &InstructionFunctions::ADD_HL_HL }
	, { "LD A (HL+)", 1, 2, &InstructionFunctions::LD_A_mHLinc }
	, { "DEC HL", 1, 2, &InstructionFunctions::DEC_HL }
	, { "INC L", 1, 1, &InstructionFunctions::INC_L }
	, { "DEC L", 1, 1, &InstructionFunctions::DEC_L }
	, { "LD L n", 2, 2, &InstructionFunctions::LD_L_n }
	, { "CPL", 1, 1, &InstructionFunctions::CPL }
	, { "JR NC n", 2, 2, &InstructionFunctions::JR_NC_n }
	, { "LD SP nn", 3, 3, &InstructionFunctions::LD_SP_nn }
	, { "LD (HL-) A", 1, 2, &InstructionFunctions::LD_mHLdec_A }
	, { "INC SP", 1, 2, &InstructionFunctions::INC_SP }
	, { "INC (HL)", 1, 3, &InstructionFunctions::INC_mHL }
	, { "DEC (HL)", 1, 3, &InstructionFunctions::DEC_mHL }
	, { "LD (HL) n", 2, 3, &InstructionFunctions::LD_mHL_n }
	, { "SCF", 1, 1, &InstructionFunctions::SCF }
	, { "JR C n", 2, 2, &InstructionFunctions::JR_C_n }
	, { "ADD HL SP", 1, 2, &InstructionFunctions::ADD_HL_SP }
	, { "LD A (HL-)", 1, 2, &InstructionFunctions::LD_A_mHLdec }
	, { "DEC SP", 1, 2, &InstructionFunctions::DEC_SP }
	, { "INC A", 1, 1, &InstructionFunctions::INC_A }
	, { "DEC_A", 1, 1, &InstructionFunctions::DEC_A }
	, { "LD A n", 2, 2, &InstructionFunctions::LD_A_n }
	, { "CCF", 1, 1, &InstructionFunctions::CCF }
	, { "LD B B", 1, 1, &InstructionFunctions::LD_B_B }
	, { "LD B C", 1, 1, &InstructionFunctions::LD_B_C }
	, { "LD B D", 1, 1, &InstructionFunctions::LD_B_D }
	, { "LD B E", 1, 1, &InstructionFunctions::LD_B_E }
	, { "LD B H", 1, 1, &InstructionFunctions::LD_B_H }
	, { "LD B L", 1, 1, &InstructionFunctions::LD_B_L }
	, { "LD B (HL)", 1, 2, &InstructionFunctions::LD_B_mHL }
	, { "LD B A", 1, 1, &InstructionFunctions::LD_B_A }
	, { "LD C B", 1, 1, &InstructionFunctions::LD_C_B }
	, { "LD C C", 1, 1, &InstructionFunctions::LD_C_C }
	, { "LD C D", 1, 1, &InstructionFunctions::LD_C_D }
	, { "LD C E", 1, 1, &InstructionFunctions::LD_C_E }
	, { "LD C H", 1, 1, &InstructionFunctions::LD_C_H }
	, { "LD C L", 1, 1, &InstructionFunctions::LD_C_L }
	, { "LD C (HL)", 1, 2, &InstructionFunctions::LD_C_mHL }
	, { "LD C A", 1, 1, &InstructionFunctions::LD_C_A }
	, { "LD D B", 1, 1, &InstructionFunctions::LD_D_B }
	, { "LD D C", 1, 1, &InstructionFunctions::LD_D_C }
	, { "LD D D", 1, 1, &InstructionFunctions::LD_D_D }
	, { "LD D E", 1, 1, &InstructionFunctions::LD_D_E }
	, { "LD D H", 1, 1, &InstructionFunctions::LD_D_H }
	, { "LD D L", 1, 1, &InstructionFunctions::LD_D_L }
	, { "LD D (HL)", 1, 2, &InstructionFunctions::LD_D_mHL }
	, { "LD D A", 1, 1, &InstructionFunctions::LD_D_A }
	, { "LD E B", 1, 1, &InstructionFunctions::LD_E_B }
	, { "LD E C", 1, 1, &InstructionFunctions::LD_E_C }
	, { "LD E D", 1, 1, &InstructionFunctions::LD_E_D }
	, { "LD E E", 1, 1, &InstructionFunctions::LD_E_E }
	, { "LD E H", 1, 1, &InstructionFunctions::LD_E_H }
	, { "LD E L", 1, 1, &InstructionFunctions::LD_E_L }
	, { "LD E (HL)", 1, 2, &InstructionFunctions::LD_E_mHL }
	, { "LD E A", 1, 1, &InstructionFunctions::LD_E_A }
	, { "LD H B", 1, 1, &InstructionFunctions::LD_H_B }
	, { "LD H C", 1, 1, &InstructionFunctions::LD_H_C }
	, { "LD H D", 1, 1, &InstructionFunctions::LD_H_D }
	, { "LD H E", 1, 1, &InstructionFunctions::LD_H_E }
	, { "LD H H", 1, 1, &InstructionFunctions::LD_H_H }
	, { "LD H L", 1, 1, &InstructionFunctions::LD_H_L }
	, { "LD H (HL)", 1, 2, &InstructionFunctions::LD_H_mHL }
	, { "LD H A", 1, 1, &InstructionFunctions::LD_H_A }
	, { "LD L B", 1, 1, &InstructionFunctions::LD_L_B }
	, { "LD L C", 1, 1, &InstructionFunctions::LD_L_C }
	, { "LD L D", 1, 1, &InstructionFunctions::LD_L_D }
	, { "LD L E", 1, 1, &InstructionFunctions::LD_L_E }
	, { "LD L H", 1, 1, &InstructionFunctions::LD_L_H }
	, { "LD L L", 1, 1, &InstructionFunctions::LD_L_L }
	, { "LD L (HL)", 1, 2, &InstructionFunctions::LD_L_mHL }
	, { "LD L A", 1, 1, &InstructionFunctions::LD_L_A }
	, { "LD (HL) B", 1, 2, &InstructionFunctions::LD_mHL_B }
	, { "LD (HL) C", 1, 2, &InstructionFunctions::LD_mHL_C }
	, { "LD (HL) D", 1, 2, &InstructionFunctions::LD_mHL_D }
	, { "LD (HL) E", 1, 2, &InstructionFunctions::LD_mHL_E }
	, { "LD (HL) H", 1, 2, &InstructionFunctions::LD_mHL_H }
	, { "LD (HL) L", 1, 2, &InstructionFunctions::LD_mHL_L }
	, { "HALT", 1, 1, &InstructionFunctions::HALT }
	, { "LD (HL) A", 1, 2, &InstructionFunctions::LD_mHL_A }
	, { "LD A B", 1, 1, &InstructionFunctions::LD_A_B }
	, { "LD A C", 1, 1, &InstructionFunctions::LD_A_C }
	, { "LD A D", 1, 1, &InstructionFunctions::LD_A_D }
	, { "LD A E", 1, 1, &InstructionFunctions::LD_A_E }
	, { "LD A H", 1, 1, &InstructionFunctions::LD_A_H }
	, { "LD A L", 1, 1, &InstructionFunctions::LD_A_L }
	, { "LD A (HL)", 1, 2, &InstructionFunctions::LD_A_mHL }
	, { "LD A A", 1, 1, &InstructionFunctions::LD_A_A }
	, { "ADD A B", 1, 1, &InstructionFunctions::ADD_A_B }
	, { "ADD A C", 1, 1, &InstructionFunctions::ADD_A_C }
	, { "ADD A D", 1, 1, &InstructionFunctions::ADD_A_D }
	, { "ADD A E", 1, 1, &InstructionFunctions::ADD_A_E }
	, { "ADD A H", 1, 1, &InstructionFunctions::ADD_A_H }
	, { "ADD A L", 1, 1, &InstructionFunctions::ADD_A_L }
	, { "ADD A (HL)", 1, 2, &InstructionFunctions::ADD_A_mHL }
	, { "ADD A A", 1, 1, &InstructionFunctions::ADD_A_A }
	, { "ADC A B", 1, 1, &InstructionFunctions::ADC_A_B }
	, { "ADC A C", 1, 1, &InstructionFunctions::ADC_A_C }
	, { "ADC A D", 1, 1, &InstructionFunctions::ADC_A_D }
	, { "ADC A E", 1, 1, &InstructionFunctions::ADC_A_E }
	, { "ADC A H", 1, 1, &InstructionFunctions::ADC_A_H }
	, { "ADC A L", 1, 1, &InstructionFunctions::ADC_A_L }
	, { "ADC A (HL)", 1, 2, &InstructionFunctions::ADC_A_mHL }
	, { "ADC A A", 1, 1, &InstructionFunctions::ADC_A_A }
	, { "SUB A B", 1, 1, &InstructionFunctions::SUB_A_B }
	, { "SUB A C", 1, 1, &InstructionFunctions::SUB_A_C }
	, { "SUB A D", 1, 1, &InstructionFunctions::SUB_A_D }
	, { "SUB A E", 1, 1, &InstructionFunctions::SUB_A_E }
	, { "SUB A H", 1, 1, &InstructionFunctions::SUB_A_H }
	, { "SUB A L", 1, 1, &InstructionFunctions::SUB_A_L }
	, { "SUB A (HL)", 1, 2, &InstructionFunctions::SUB_A_mHL }
	, { "SUB A A", 1, 1, &InstructionFunctions::SUB_A_A }
	, { "SBC A B", 1, 1, &InstructionFunctions::SBC_A_B }
	, { "SBC A C", 1, 1, &InstructionFunctions::SBC_A_C }
	, { "SBC A D", 1, 1, &InstructionFunctions::SBC_A_D }
	, { "SBC A E", 1, 1, &InstructionFunctions::SBC_A_E }
	, { "SBC A H", 1, 1, &InstructionFunctions::SBC_A_H }
	, { "SBC A L", 1, 1, &InstructionFunctions::SBC_A_L }
	, { "SBC A (HL)", 1, 2, &InstructionFunctions::SBC_A_mHL }
	, { "SBC A A", 1, 1, &InstructionFunctions::SBC_A_A }
	, { "AND A B", 1, 1, &InstructionFunctions::AND_A_B }
	, { "AND A C", 1, 1, &InstructionFunctions::AND_A_C }
	, { "AND A D", 1, 1, &InstructionFunctions::AND_A_D }
	, { "AND A E", 1, 1, &InstructionFunctions::AND_A_E }
	, { "AND A H", 1, 1, &InstructionFunctions::AND_A_H }
	, { "AND A L", 1, 1, &InstructionFunctions::AND_A_L }
	, { "AND A (HL)", 1, 2, &InstructionFunctions::AND_A_mHL }
	, { "AND A A", 1, 1, &InstructionFunctions::AND_A_A }
	, { "XOR A B", 1, 1, &InstructionFunctions::XOR_A_B }
	, { "XOR A B", 1, 1, &InstructionFunctions::XOR_A_C }
	, { "XOR A B", 1, 1, &InstructionFunctions::XOR_A_D }
	, { "XOR A B", 1, 1, &InstructionFunctions::XOR_A_E }
	, { "XOR A B", 1, 1, &InstructionFunctions::XOR_A_H }
	, { "XOR A B", 1, 1, &InstructionFunctions::XOR_A_L }
	, { "XOR A B", 1, 2, &InstructionFunctions::XOR_A_mHL }
	, { "XOR A B", 1, 1, &InstructionFunctions::XOR_A_A }
	, { "OR A B", 1, 1, &InstructionFunctions::OR_A_B }
	, { "OR A C", 1, 1, &InstructionFunctions::OR_A_C }
	, { "OR A D", 1, 1, &InstructionFunctions::OR_A_D }
	, { "OR A E", 1, 1, &InstructionFunctions::OR_A_E }
	, { "OR A H", 1, 1, &InstructionFunctions::OR_A_H }
	, { "OR A L", 1, 1, &InstructionFunctions::OR_A_L }
	, { "OR A (HL)", 1, 2, &InstructionFunctions::OR_A_mHL }
	, { "OR A A", 1, 1, &InstructionFunctions::OR_A_A }
	, { "CP A B", 1, 1, &InstructionFunctions::CP_A_B }
	, { "CP A C", 1, 1, &InstructionFunctions::CP_A_C }
	, { "CP A D", 1, 1, &InstructionFunctions::CP_A_D }
	, { "CP A E", 1, 1, &InstructionFunctions::CP_A_E }
	, { "CP A H", 1, 1, &InstructionFunctions::CP_A_H }
	, { "CP A L", 1, 1, &InstructionFunctions::CP_A_L }
	, { "CP A (HL)", 1, 2, &InstructionFunctions::CP_A_mHL }
	, { "CP A A", 1, 1, &InstructionFunctions::CP_A_A }
	, { "RET NZ", 1, 5, &InstructionFunctions::RET_NZ }
	, { "POP BC", 1, 3, &InstructionFunctions::POP_BC }
	, { "JP NZ nn", 3, 3, &InstructionFunctions::JP_NZ_nn }
	, { "JP nn", 3, 4, &InstructionFunctions::JP_nn }
	, { "CALL NZ nn", 3, 6, &InstructionFunctions::CALL_NZ_nn }
	, { "PUSH BC", 1, 4, &InstructionFunctions::PUSH_BC }
	, { "ADD A n", 2, 2, &InstructionFunctions::ADD_A_n }
	, { "RST 0x00", 1, 4, &InstructionFunctions::RST_00 }
	, { "RET Z", 1, 5, &InstructionFunctions::RET_Z }
	, { "RET", 1, 4, &InstructionFunctions::RET }
	, { "JP Z nn", 3, 3, &InstructionFunctions::JP_Z_nn }
	, { "CB OP", 1, 1, &InstructionFunctions::UNIMPLEMENTED }
	, { "CALL Z nn", 3, 6, &InstructionFunctions::CALL_Z_nn }
	, { "CALL nn", 3, 6, &InstructionFunctions::CALL_nn }
	, { "ADC A n", 2, 2, &InstructionFunctions::ADC_A_n }
	, { "RST 0x08", 1, 4, &InstructionFunctions::RST_08 }
	, { "RET NC", 1, 5, &InstructionFunctions::RET_NC }
	, { "POP DE", 1, 3, &InstructionFunctions::POP_DE }
	, { "JP NC nn", 3, 3, &InstructionFunctions::JP_NC_nn }
	, { "UNASSIGNED", 1, 1, &InstructionFunctions::UNIMPLEMENTED }
	, { "CALL NC nn", 3, 6, &InstructionFunctions::CALL_NC_nn }
	, { "PUSH DE", 1, 4, &InstructionFunctions::PUSH_DE }
	, { "SUB A n", 2, 2, &InstructionFunctions::SUB_A_n }
	, { "RST 0x10", 1, 4, &InstructionFunctions::RST_10 }
	, { "RET C", 1, 5, &InstructionFunctions::RET_C }
	, { "RETI", 1, 4, &InstructionFunctions::RETI }
	, { "JP C nn", 3, 3, &InstructionFunctions::JP_C_nn }
	, { "UNASSIGNED", 1, 1, &InstructionFunctions::UNIMPLEMENTED }
	, { "CALL C nn", 3, 6, &InstructionFunctions::CALL_C_nn }
	, { "UNASSIGNED", 1, 1, &InstructionFunctions::UNIMPLEMENTED }
	, { "SBC A n", 2, 2, &InstructionFunctions::SBC_A_n }
	, { "RST 0x18", 1, 4, &InstructionFunctions::RST_18 }
	, { "LDH (n) A", 2, 3, &InstructionFunctions::LDH_mn_A }
	, { "POP HL", 1, 3, &InstructionFunctions::POP_HL }
	, { "LDH (C) A", 1, 2, &InstructionFunctions::LDH_mC_A }
	, { "UNASSIGNED", 1, 1, &InstructionFunctions::UNIMPLEMENTED }
	, { "UNASSIGNED", 1, 1, &InstructionFunctions::UNIMPLEMENTED }
	, { "PUSH HL", 1, 4, &InstructionFunctions::PUSH_HL }
	, { "AND A n", 2, 2, &InstructionFunctions::AND_A_n }
	, { "RST 0x20", 1, 4, &InstructionFunctions::RST_20 }
	, { "ADD SP n", 2, 4, &InstructionFunctions::ADD_SP_n }
	, { "JP HL", 1, 1, &InstructionFunctions::JP_HL }
	, { "LD (nn) A", 3, 4, &InstructionFunctions::LD_mnn_A }
	, { "UNASSIGNED", 1, 1, &InstructionFunctions::UNIMPLEMENTED }
	, { "UNASSIGNED", 1, 1, &InstructionFunctions::UNIMPLEMENTED }
	, { "UNASSIGNED", 1, 1, &InstructionFunctions::UNIMPLEMENTED }
	, { "XOR A n", 2, 2, &InstructionFunctions::XOR_A_n }
	, { "RST 0x28", 1, 4, &InstructionFunctions::RST_28 }
	, { "LDH A (n)", 2, 3, &InstructionFunctions::LDH_A_mn }
	, { "POP AF", 1, 3, &InstructionFunctions::POP_AF }
	, { "LDH A (C)", 1, 2, &InstructionFunctions::LDH_A_mC }
	, { "DI", 1, 1, &InstructionFunctions::DI }
	, { "UNASSIGNED", 1, 1, &InstructionFunctions::UNIMPLEMENTED }
	, { "PUSH AF", 1, 4, &InstructionFunctions::PUSH_AF }
	, { "OR A n", 2, 2, &InstructionFunctions::OR_A_n }
	, { "RST 0x30", 1, 4, &InstructionFunctions::RST_30 }
	, { "LD HL SP+n", 2, 3, &InstructionFunctions::LD_HL_SP_n }
	, { "LD SP HL", 1, 2, &InstructionFunctions::LD_SP_HL }
	, { "LD A (nn)", 3, 4, &InstructionFunctions::LD_A_mnn }
	, { "EI", 1, 1, &InstructionFunctions::EI }
	, { "UNASSIGNED", 1, 1, &InstructionFunctions::UNIMPLEMENTED }
	, { "UNASSIGNED", 1, 1, &InstructionFunctions::UNIMPLEMENTED }
	, { "CP A n", 2, 2, &InstructionFunctions::CP_A_n }
	, { "RST 0x38", 1, 4, &InstructionFunctions::RST_38 }
	, { "RLC B", 2, 2, &InstructionFunctions::RLC_B }
	, { "RLC C", 2, 2, &InstructionFunctions::RLC_C }
	, { "RLC D", 2, 2, &InstructionFunctions::RLC_D }
	, { "RLC E", 2, 2, &InstructionFunctions::RLC_E }
	, { "RLC H", 2, 2, &InstructionFunctions::RLC_H }
	, { "RLC L", 2, 2, &InstructionFunctions::RLC_L }
	, { "RLC (HL)", 2, 4, &InstructionFunctions::RLC_mHL }
	, { "RLC A", 2, 2, &InstructionFunctions::RLC_A }
	, { "RRC B", 2, 2, &InstructionFunctions::RRC_B }
	, { "RRC C", 2, 2, &InstructionFunctions::RRC_C }
	, { "RRC D", 2, 2, &InstructionFunctions::RRC_D }
	, { "RRC E", 2, 2, &InstructionFunctions::RRC_E }
	, { "RRC H", 2, 2, &InstructionFunctions::RRC_H }
	, { "RRC L", 2, 2, &InstructionFunctions::RRC_L }
	, { "RRC (HL)", 2, 4, &InstructionFunctions::RRC_mHL }
	, { "RRC A", 2, 2, &InstructionFunctions::RRC_A }
	, { "RL B", 2, 2, &InstructionFunctions::RL_B }
	, { "RL C", 2, 2, &InstructionFunctions::RL_C }
	, { "RL D", 2, 2, &InstructionFunctions::RL_D }
	, { "RL E", 2, 2, &InstructionFunctions::RL_E }
	, { "RL H", 2, 2, &InstructionFunctions::RL_H }
	, { "RL L", 2, 2, &InstructionFunctions::RL_L }
	, { "RL (HL)", 2, 4, &InstructionFunctions::RL_mHL }
	, { "RL A", 2, 2, &InstructionFunctions::RL_A }
	, { "RR B", 2, 2, &InstructionFunctions::RR_B }
	, { "RR C", 2, 2, &InstructionFunctions::RR_C }
	, { "RR D", 2, 2, &InstructionFunctions::RR_D }
	, { "RR E", 2, 2, &InstructionFunctions::RR_E }
	, { "RR H", 2, 2, &InstructionFunctions::RR_H }
	, { "RR L", 2, 2, &InstructionFunctions::RR_L }
	, { "RR (HL)", 2, 4, &InstructionFunctions::RR_mHL }
	, { "RR A", 2, 2, &InstructionFunctions::RR_A }
	, { "SLA B", 2, 2, &InstructionFunctions::SLA_B }
	, { "SLA C", 2, 2, &InstructionFunctions::SLA_C }
	, { "SLA D", 2, 2, &InstructionFunctions::SLA_D }
	, { "SLA E", 2, 2, &InstructionFunctions::SLA_E }
	, { "SLA H", 2, 2, &InstructionFunctions::SLA_H }
	, { "SLA L", 2, 2, &InstructionFunctions::SLA_L }
	, { "SLA (HL)", 2, 4, &InstructionFunctions::SLA_mHL }
	, { "SLA A", 2, 2, &InstructionFunctions::SLA_A }
	, { "SRA B", 2, 2, &InstructionFunctions::SRA_B }
	, { "SRA C", 2, 2, &InstructionFunctions::SRA_C }
	, { "SRA D", 2, 2, &InstructionFunctions::SRA_D }
	, { "SRA E", 2, 2, &InstructionFunctions::SRA_E }
	, { "SRA H", 2, 2, &InstructionFunctions::SRA_H }
	, { "SRA L", 2, 2, &InstructionFunctions::SRA_L }
	, { "SRA (HL)", 2, 4, &InstructionFunctions::SRA_mHL }
	, { "SRA A", 2, 2, &InstructionFunctions::SRA_A }
	, { "SWAP B", 2, 2, &InstructionFunctions::SWAP_B }
	, { "SWAP C", 2, 2, &InstructionFunctions::SWAP_C }
	, { "SWAP D", 2, 2, &InstructionFunctions::SWAP_D }
	, { "SWAP E", 2, 2, &InstructionFunctions::SWAP_E }
	, { "SWAP H", 2, 2, &InstructionFunctions::SWAP_H }
	, { "SWAP L", 2, 2, &InstructionFunctions::SWAP_L }
	, { "SWAP (HL)", 2, 4, &InstructionFunctions::SWAP_mHL }
	, { "SWAP A", 2, 2, &InstructionFunctions::SWAP_A }
	, { "SRL B", 2, 2, &InstructionFunctions::SRL_B }
	, { "SRL C", 2, 2, &InstructionFunctions::SRL_C }
	, { "SRL D", 2, 2, &InstructionFunctions::SRL_D }
	, { "SRL E", 2, 2, &InstructionFunctions::SRL_E }
	, { "SRL H", 2, 2, &InstructionFunctions::SRL_H }
	, { "SRL L", 2, 2, &InstructionFunctions::SRL_L }
	, { "SRL (HL)", 2, 4, &InstructionFunctions::SRL_mHL }
	, { "SRL A", 2, 2, &InstructionFunctions::SRL_A }
	, { "BIT 0 B", 2, 2, &InstructionFunctions::BIT_0_B }
	, { "BIT 0 C", 2, 2, &InstructionFunctions::BIT_0_C }
	, { "BIT 0 D", 2, 2, &InstructionFunctions::BIT_0_D }
	, { "BIT 0 E", 2, 2, &InstructionFunctions::BIT_0_E }
	, { "BIT 0 H", 2, 2, &InstructionFunctions::BIT_0_H }
	, { "BIT 0 L", 2, 2, &InstructionFunctions::BIT_0_L }
	, { "BIT 0 (HL)", 2, 3, &InstructionFunctions::BIT_0_mHL }
	, { "BIT 0 A", 2, 2, &InstructionFunctions::BIT_0_A }
	, { "BIT 1 B", 2, 2, &InstructionFunctions::BIT_1_B }
	, { "BIT 1 C", 2, 2, &InstructionFunctions::BIT_1_C }
	, { "BIT 1 D", 2, 2, &InstructionFunctions::BIT_1_D }
	, { "BIT 1 E", 2, 2, &InstructionFunctions::BIT_1_E }
	, { "BIT 1 H", 2, 2, &InstructionFunctions::BIT_1_H }
	, { "BIT 1 L", 2, 2, &InstructionFunctions::BIT_1_L }
	, { "BIT 1 (HL)", 2, 3, &InstructionFunctions::BIT_1_mHL }
	, { "BIT 1 A", 2, 2, &InstructionFunctions::BIT_1_A }
	, { "BIT 2 B", 2, 2, &InstructionFunctions::BIT_2_B }
	, { "BIT 2 C", 2, 2, &InstructionFunctions::BIT_2_C }
	, { "BIT 2 D", 2, 2, &InstructionFunctions::BIT_2_D }
	, { "BIT 2 E", 2, 2, &InstructionFunctions::BIT_2_E }
	, { "BIT 2 H", 2, 2, &InstructionFunctions::BIT_2_H }
	, { "BIT 2 L", 2, 2, &InstructionFunctions::BIT_2_L }
	, { "BIT 2 (HL)", 2, 3, &InstructionFunctions::BIT_2_mHL }
	, { "BIT 2 A", 2, 2, &InstructionFunctions::BIT_2_A }
	, { "BIT 3 B", 2, 2, &InstructionFunctions::BIT_3_B }
	, { "BIT 3 C", 2, 2, &InstructionFunctions::BIT_3_C }
	, { "BIT 3 D", 2, 2, &InstructionFunctions::BIT_3_D }
	, { "BIT 3 E", 2, 2, &InstructionFunctions::BIT_3_E }
	, { "BIT 3 H", 2, 2, &InstructionFunctions::BIT_3_H }
	, { "BIT 3 L", 2, 2, &InstructionFunctions::BIT_3_L }
	, { "BIT 3 (HL)", 2, 3, &InstructionFunctions::BIT_3_mHL }
	, { "BIT 3 A", 2, 2, &InstructionFunctions::BIT_3_A }
	, { "BIT 4 B", 2, 2, &InstructionFunctions::BIT_4_B }
	, { "BIT 4 C", 2, 2, &InstructionFunctions::BIT_4_C }
	, { "BIT 4 D", 2, 2, &InstructionFunctions::BIT_4_D }
	, { "BIT 4 E", 2, 2, &InstructionFunctions::BIT_4_E }
	, { "BIT 4 H", 2, 2, &InstructionFunctions::BIT_4_H }
	, { "BIT 4 L", 2, 2, &InstructionFunctions::BIT_4_L }
	, { "BIT 4 (HL)", 2, 3, &InstructionFunctions::BIT_4_mHL }
	, { "BIT 4 A", 2, 2, &InstructionFunctions::BIT_4_A }
	, { "BIT 5 B", 2, 2, &InstructionFunctions::BIT_5_B }
	, { "BIT 5 C", 2, 2, &InstructionFunctions::BIT_5_C }
	, { "BIT 5 D", 2, 2, &InstructionFunctions::BIT_5_D }
	, { "BIT 5 E", 2, 2, &InstructionFunctions::BIT_5_E }
	, { "BIT 5 H", 2, 2, &InstructionFunctions::BIT_5_H}
	, { "BIT 5 L", 2, 2, &InstructionFunctions::BIT_5_L}
	, { "BIT 5 (HL)", 2, 3, &InstructionFunctions::BIT_5_mHL }
	, { "BIT 5 A", 2, 2, &InstructionFunctions::BIT_5_A }
	, { "BIT 6 B", 2, 2, &InstructionFunctions::BIT_6_B }
	, { "BIT 6 C", 2, 2, &InstructionFunctions::BIT_6_C }
	, { "BIT 6 D", 2, 2, &InstructionFunctions::BIT_6_D }
	, { "BIT 6 E", 2, 2, &InstructionFunctions::BIT_6_E }
	, { "BIT 6 H", 2, 2, &InstructionFunctions::BIT_6_H }
	, { "BIT 6 L", 2, 2, &InstructionFunctions::BIT_6_L}
	, { "BIT 6 (HL)", 2, 3, &InstructionFunctions::BIT_6_mHL }
	, { "BIT 6 A", 2, 2, &InstructionFunctions::BIT_6_A }
	, { "BIT 7 B", 2, 2, &InstructionFunctions::BIT_7_B }
	, { "BIT 7 C", 2, 2, &InstructionFunctions::BIT_7_C }
	, { "BIT 7 D", 2, 2, &InstructionFunctions::BIT_7_D }
	, { "BIT 7 E", 2, 2, &InstructionFunctions::BIT_7_E }
	, { "BIT 7 H", 2, 2, &InstructionFunctions::BIT_7_H }
	, { "BIT 7 L", 2, 2, &InstructionFunctions::BIT_7_L }
	, { "BIT 7 (HL)", 2, 3, &InstructionFunctions::BIT_7_mHL }
	, { "BIT 7 A", 2, 2, &InstructionFunctions::BIT_7_A }
	, { "RES 0 B", 2, 2, &InstructionFunctions::RES_0_B }
	, { "RES 0 C", 2, 2, &InstructionFunctions::RES_0_C }
	, { "RES 0 D", 2, 2, &InstructionFunctions::RES_0_D }
	, { "RES 0 E", 2, 2, &InstructionFunctions::RES_0_E }
	, { "RES 0 H", 2, 2, &InstructionFunctions::RES_0_H }
	, { "RES 0 L", 2, 2, &InstructionFunctions::RES_0_L }
	, { "RES 0 (HL)", 2, 4, &InstructionFunctions::RES_0_mHL }
	, { "RES 0 A", 2, 2, &InstructionFunctions::RES_0_A }
	, { "RES 1 B", 2, 2, &InstructionFunctions::RES_1_B }
	, { "RES 1 C", 2, 2, &InstructionFunctions::RES_1_C }
	, { "RES 1 D", 2, 2, &InstructionFunctions::RES_1_D }
	, { "RES 1 E", 2, 2, &InstructionFunctions::RES_1_E }
	, { "RES 1 H", 2, 2, &InstructionFunctions::RES_1_H }
	, { "RES 1 L", 2, 2, &InstructionFunctions::RES_1_L }
	, { "RES 1 (HL)", 2, 4, &InstructionFunctions::RES_1_mHL }
	, { "RES 1 A", 2, 2, &InstructionFunctions::RES_1_A }
	, { "RES 2 B", 2, 2, &InstructionFunctions::RES_2_B }
	, { "RES 2 C", 2, 2, &InstructionFunctions::RES_2_C }
	, { "RES 2 D", 2, 2, &InstructionFunctions::RES_2_D }
	, { "RES 2 E", 2, 2, &InstructionFunctions::RES_2_E }
	, { "RES 2 H", 2, 2, &InstructionFunctions::RES_2_H }
	, { "RES 2 L", 2, 2, &InstructionFunctions::RES_2_L }
	, { "RES 2 (HL)", 2, 4, &InstructionFunctions::RES_2_mHL }
	, { "RES 2 A", 2, 2, &InstructionFunctions::RES_2_A }
	, { "RES 3 B", 2, 2, &InstructionFunctions::RES_3_B }
	, { "RES 3 C", 2, 2, &InstructionFunctions::RES_3_C }
	, { "RES 3 D", 2, 2, &InstructionFunctions::RES_3_D }
	, { "RES 3 E", 2, 2, &InstructionFunctions::RES_3_E }
	, { "RES 3 H", 2, 2, &InstructionFunctions::RES_3_H }
	, { "RES 3 L", 2, 2, &InstructionFunctions::RES_3_L }
	, { "RES 3 (HL)", 2, 4, &InstructionFunctions::RES_3_mHL }
	, { "RES 3 A", 2, 2, &InstructionFunctions::RES_3_A }
	, { "RES 4 B", 2, 2, &InstructionFunctions::RES_4_B }
	, { "RES 4 C", 2, 2, &InstructionFunctions::RES_4_C }
	, { "RES 4 D", 2, 2, &InstructionFunctions::RES_4_D }
	, { "RES 4 E", 2, 2, &InstructionFunctions::RES_4_E }
	, { "RES 4 H", 2, 2, &InstructionFunctions::RES_4_H }
	, { "RES 4 L", 2, 2, &InstructionFunctions::RES_4_L }
	, { "RES 4 (HL)", 2, 4, &InstructionFunctions::RES_4_mHL }
	, { "RES 4 A", 2, 2, &InstructionFunctions::RES_4_A }
	, { "RES 5 B", 2, 2, &InstructionFunctions::RES_5_B }
	, { "RES 5 C", 2, 2, &InstructionFunctions::RES_5_C }
	, { "RES 5 D", 2, 2, &InstructionFunctions::RES_5_D }
	, { "RES 5 E", 2, 2, &InstructionFunctions::RES_5_E }
	, { "RES 5 H", 2, 2, &InstructionFunctions::RES_5_H }
	, { "RES 5 L", 2, 2, &InstructionFunctions::RES_5_L }
	, { "RES 5 (HL)", 2, 4, &InstructionFunctions::RES_5_mHL }
	, { "RES 5 A", 2, 2, &InstructionFunctions::RES_5_A }
	, { "RES 6 B", 2, 2, &InstructionFunctions::RES_6_B }
	, { "RES 6 C", 2, 2, &InstructionFunctions::RES_6_C }
	, { "RES 6 D", 2, 2, &InstructionFunctions::RES_6_D }
	, { "RES 6 E", 2, 2, &InstructionFunctions::RES_6_E }
	, { "RES 6 H", 2, 2, &InstructionFunctions::RES_6_H }
	, { "RES 6 L", 2, 2, &InstructionFunctions::RES_6_L }
	, { "RES 6 (HL)", 2, 4, &InstructionFunctions::RES_6_mHL }
	, { "RES 6 A", 2, 2, &InstructionFunctions::RES_6_A }
	, { "RES 7 B", 2, 2, &InstructionFunctions::RES_7_B }
	, { "RES 7 C", 2, 2, &InstructionFunctions::RES_7_C }
	, { "RES 7 D", 2, 2, &InstructionFunctions::RES_7_D }
	, { "RES 7 E", 2, 2, &InstructionFunctions::RES_7_E }
	, { "RES 7 H", 2, 2, &InstructionFunctions::RES_7_H }
	, { "RES 7 L", 2, 2, &InstructionFunctions::RES_7_L }
	, { "RES 7 (HL)", 2, 4, &InstructionFunctions::RES_7_mHL }
	, { "RES 7 A", 2, 2, &InstructionFunctions::RES_7_A }	
	, { "SET 0 B", 2, 2, &InstructionFunctions::SET_0_B }
	, { "SET 0 C", 2, 2, &InstructionFunctions::SET_0_C }
	, { "SET 0 D", 2, 2, &InstructionFunctions::SET_0_D }
	, { "SET 0 E", 2, 2, &InstructionFunctions::SET_0_E }
	, { "SET 0 H", 2, 2, &InstructionFunctions::SET_0_H }
	, { "SET 0 L", 2, 2, &InstructionFunctions::SET_0_L }
	, { "SET 0 (HL)", 2, 4, &InstructionFunctions::SET_0_mHL }
	, { "SET 0 A", 2, 2, &InstructionFunctions::SET_0_A }
	, { "SET 1 B", 2, 2, &InstructionFunctions::SET_1_B }
	, { "SET 1 C", 2, 2, &InstructionFunctions::SET_1_C }
	, { "SET 1 D", 2, 2, &InstructionFunctions::SET_1_D }
	, { "SET 1 E", 2, 2, &InstructionFunctions::SET_1_E }
	, { "SET 1 H", 2, 2, &InstructionFunctions::SET_1_H }
	, { "SET 1 L", 2, 2, &InstructionFunctions::SET_1_L }
	, { "SET 1 (HL)", 2, 4, &InstructionFunctions::SET_1_mHL }
	, { "SET 1 A", 2, 2, &InstructionFunctions::SET_1_A }
	, { "SET 2 B", 2, 2, &InstructionFunctions::SET_2_B }
	, { "SET 2 C", 2, 2, &InstructionFunctions::SET_2_C }
	, { "SET 2 D", 2, 2, &InstructionFunctions::SET_2_D }
	, { "SET 2 E", 2, 2, &InstructionFunctions::SET_2_E }
	, { "SET 2 H", 2, 2, &InstructionFunctions::SET_2_H }
	, { "SET 2 L", 2, 2, &InstructionFunctions::SET_2_L }
	, { "SET 2 (HL)", 2, 4, &InstructionFunctions::SET_2_mHL }
	, { "SET 2 A", 2, 2, &InstructionFunctions::SET_2_A }
	, { "SET 3 B", 2, 2, &InstructionFunctions::SET_3_B }
	, { "SET 3 C", 2, 2, &InstructionFunctions::SET_3_C }
	, { "SET 3 D", 2, 2, &InstructionFunctions::SET_3_D }
	, { "SET 3 E", 2, 2, &InstructionFunctions::SET_3_E }
	, { "SET 3 H", 2, 2, &InstructionFunctions::SET_3_H }
	, { "SET 3 L", 2, 2, &InstructionFunctions::SET_3_L }
	, { "SET 3 (HL)", 2, 4, &InstructionFunctions::SET_3_mHL }
	, { "SET 3 A", 2, 2, &InstructionFunctions::SET_3_A }
	, { "SET 4 B", 2, 2, &InstructionFunctions::SET_4_B }
	, { "SET 4 C", 2, 2, &InstructionFunctions::SET_4_C }
	, { "SET 4 D", 2, 2, &InstructionFunctions::SET_4_D }
	, { "SET 4 E", 2, 2, &InstructionFunctions::SET_4_E }
	, { "SET 4 H", 2, 2, &InstructionFunctions::SET_4_H }
	, { "SET 4 L", 2, 2, &InstructionFunctions::SET_4_L }
	, { "SET 4 (HL)", 2, 4, &InstructionFunctions::SET_4_mHL }
	, { "SET 4 A", 2, 2, &InstructionFunctions::SET_4_A }
	, { "SET 5 B", 2, 2, &InstructionFunctions::SET_5_B }
	, { "SET 5 C", 2, 2, &InstructionFunctions::SET_5_C }
	, { "SET 5 D", 2, 2, &InstructionFunctions::SET_5_D }
	, { "SET 5 E", 2, 2, &InstructionFunctions::SET_5_E }
	, { "SET 5 H", 2, 2, &InstructionFunctions::SET_5_H }
	, { "SET 5 L", 2, 2, &InstructionFunctions::SET_5_L }
	, { "SET 5 (HL)", 2, 4, &InstructionFunctions::SET_5_mHL }
	, { "SET 5 A", 2, 2, &InstructionFunctions::SET_5_A }
	, { "SET 6 B", 2, 2, &InstructionFunctions::SET_6_B }
	, { "SET 6 C", 2, 2, &InstructionFunctions::SET_6_C }
	, { "SET 6 D", 2, 2, &InstructionFunctions::SET_6_D }
	, { "SET 6 E", 2, 2, &InstructionFunctions::SET_6_E }
	, { "SET 6 H", 2, 2, &InstructionFunctions::SET_6_H }
	, { "SET 6 L", 2, 2, &InstructionFunctions::SET_6_L }
	, { "SET 6 (HL)", 2, 4, &InstructionFunctions::SET_6_mHL }
	, { "SET 6 A", 2, 2, &InstructionFunctions::SET_6_A }
	, { "SET 7 B", 2, 2, &InstructionFunctions::SET_7_B }
	, { "SET 7 C", 2, 2, &InstructionFunctions::SET_7_C }
	, { "SET 7 D", 2, 2, &InstructionFunctions::SET_7_D }
	, { "SET 7 E", 2, 2, &InstructionFunctions::SET_7_E }
	, { "SET 7 H", 2, 2, &InstructionFunctions::SET_7_H }
	, { "SET 7 L", 2, 2, &InstructionFunctions::SET_7_L }
	, { "SET 7 (HL)", 2, 4, &InstructionFunctions::SET_7_mHL }
	, { "SET 7 A", 2, 2, &InstructionFunctions::SET_7_A }
}
{
}