#include "Joypad.h"
#include "Helpers.h"

#define P1_REGISTER 0xFF00

FORCE_INLINE bool IsActionGroupSelected(uint8_t upperNibble)
{
	return (upperNibble & 0x20) > 0;
}

void Joypad::Init(Memory& memory)
{
	memory.Write(P1_REGISTER, 0xCF);
	m_dPad = 0x0F;
	m_Buttons = 0x0F;
}

void Joypad::Update(Memory& memory)
{
	uint8_t upperNibble = memory[P1_REGISTER] & 0xF0;
	if (IsActionGroupSelected(upperNibble))
	{
		memory.Write(P1_REGISTER, upperNibble | m_Buttons);
	}
	else
	{
		memory.Write(P1_REGISTER, upperNibble | m_dPad);
	}
}
