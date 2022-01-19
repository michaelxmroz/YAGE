#include "Joypad.h"
#include "Helpers.h"

#define P1_REGISTER 0xFF00

FORCE_INLINE bool IsActionGroupSelected(uint8_t upperNibble)
{
	return (upperNibble & 0x20) > 0;
}

Joypad::Joypad()
{
}

void Joypad::Init(Memory& memory)
{
	memory.Write(P1_REGISTER, 0xCF);
}

void Joypad::Update(EmulatorInputs::InputState state, Memory& memory)
{
	uint8_t upperNibble = memory[P1_REGISTER] & 0xF0;
	if (IsActionGroupSelected(upperNibble))
	{
		memory.Write(P1_REGISTER, upperNibble | state.m_buttons);
	}
	else
	{
		memory.Write(P1_REGISTER, upperNibble | state.m_dPad);
	}
}
