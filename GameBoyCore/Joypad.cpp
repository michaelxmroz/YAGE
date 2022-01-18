#include "Joypad.h"
#include "Helpers.h"

#define P1_REGISTER 0xFF00

FORCE_INLINE bool IsActionGroupSelected(uint8_t upperNibble)
{
	return (upperNibble & 0x20) > 0;
}

Joypad::Joypad() : m_queryInput(nullptr), m_state()
{
}

void Joypad::Init(Memory& memory)
{
	memory.Write(P1_REGISTER, 0xCF);
	m_state.m_dPad = 0x0F;
	m_state.m_buttons = 0x0F;
}

void Joypad::SetInputFunction(JoypadFunc func)
{
	m_queryInput = func;
}

void Joypad::Update(Memory& memory)
{
	if (m_queryInput != nullptr) 
	{
		m_state = m_queryInput();
	}

	uint8_t upperNibble = memory[P1_REGISTER] & 0xF0;
	if (IsActionGroupSelected(upperNibble))
	{
		memory.Write(P1_REGISTER, upperNibble | m_state.m_buttons);
	}
	else
	{
		memory.Write(P1_REGISTER, upperNibble | m_state.m_dPad);
	}
}
