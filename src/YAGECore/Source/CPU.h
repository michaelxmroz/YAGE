#pragma once
#include "Memory.h"
#include "Registers.h"
#include "InstructionFunctions.h"
#include "Interrupts.h"
#include "CppIncludes.h"
#include "../Include/Emulator.h"

#define INSTRUCTION_SET_SIZE 513

class CPU : ISerializable
{
public:
	CPU();
	explicit CPU(bool enableInterruptHandling);
	explicit CPU(GamestateSerializer* serializer);
	CPU(GamestateSerializer* serializer, bool enableInterruptHandling);
	~CPU();

#if _DEBUG
	void SetInstructionCallback(uint8_t instr, Emulator::DebugCallback callback, void* userData);
	void SetInstructionCountCallback(uint64_t instrCount, Emulator::DebugCallback callback, void* userData);
	void SetPCCallback(uint16_t pc, Emulator::DebugCallback callback, void* userData);
	void ClearCallbacks();

	Emulator::CPUState GetCPUState() const;

	// Returns disassembly info for an instruction at the given address
	Emulator::DisassemblyInfo GetDisassemblyInfo(uint16_t addr, Memory& memory) const;

	// Disassembles the ROM banks (0x0000-0x7FFF)
	void DisassembleROM(Memory& memory);

#endif
#if _TESTING
	void StopOnInstruction(uint8_t instr);
	bool HasReachedInstruction(uint8_t instr);
	Registers& GetRegisters()
	{
		return m_registers;
	}
#endif
	uint32_t Step(Memory& memory);

	void SetProgramCounter(unsigned short addr);

	void Reset();
	void ResetToBootromValues();

private:
	typedef InstructionResult (*InstructionFunc)(const char* mnemonic, InstructionTempData& data, Registers* registers, Memory& memory);

	struct Instruction
	{
		const char* m_mnemonic;
		const uint8_t m_length;
		const uint8_t m_duration;
		const InstructionFunc m_func;
	};

	void ClearRegisters();
	void ExecuteInstruction(Memory& memory);
	void DecodeAndFetchNext(Memory& memory);
	bool ProcessInterrupts(Memory& memory);
	bool CheckForWakeup(Memory& memory, bool postFetch);

	void Serialize(uint8_t* data) override;
	void Deserialize(const uint8_t* data) override;
	virtual uint32_t GetSerializationSize() override;

	Registers m_registers;

	const Instruction m_instructions[INSTRUCTION_SET_SIZE + 1];

	bool m_delayedInterruptHandling;
	const bool m_InterruptHandlingEnabled;

	const Instruction* m_currentInstruction;
	InstructionTempData m_instructionTempData;
	bool m_isNextInstructionCB;

#if _DEBUG

	std::map<uint16_t, Emulator::DebugCallback> DEBUG_PCCallbackMap;
	std::map<uint8_t, Emulator::DebugCallback> DEBUG_instrCallbackMap;
	std::map<uint64_t, Emulator::DebugCallback> DEBUG_instrCountCallbackMap;

	std::map<uint16_t, void*> DEBUG_PCCallbackUserData;
	std::map<uint8_t, void*> DEBUG_instrCallbackUserData;
	std::map<uint64_t, void*> DEBUG_instrCountCallbackUserData;



	uint64_t DEBUG_instructionCount = 0;

#if CPU_STATE_LOGGING
	char* DEBUG_CPUInstructionLog;
	const char* DEBUG_LogTemplate = "A:00 F:00 B:00 C:00 D:00 E:00 H:00 L:00 SP:0000 PC:0000 PCMEM:00,00,00,00 IO:00 CYC:0 OP:           \n"; 
#endif // CPU_STATE_LOGGING
#endif
#if _TESTING
	std::map<uint8_t, bool> DEBUG_stopInstructions;
#endif

#if _DEBUG
	// Disassembly cache
	uint8_t* m_disasmMap;
	bool m_disasmMapValid;
#endif
};

