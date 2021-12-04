#pragma once
#include <cstdint>
#include "Registers.h"
#include "Logging.h"

namespace InstructionFunctions
{
	void LoadToRegister(const char* mnemonic, uint8_t length, Registers* registers, uint8_t* memory);
}

