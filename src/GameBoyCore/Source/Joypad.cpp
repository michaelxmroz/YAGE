#include "Joypad.h"
#include "Helpers.h"

#define P1_REGISTER 0xFF00

FORCE_INLINE bool IsActionGroupSelected(uint8_t upperNibble)
{
	return (upperNibble & 0x20) == 0;
}

Joypad::Joypad()
{
}

void Joypad::Init(Memory& memory)
{
	memory.Write(P1_REGISTER, 0xCF);
	memory.RegisterCallback(P1_REGISTER, Joypad::ToggleGroup, nullptr);
}

void Joypad::Update(EmulatorInputs::InputState state, Memory& memory)
{
	uint8_t upperNibble = memory[P1_REGISTER] & 0xF0;
	if (IsActionGroupSelected(upperNibble))
	{
		memory.WriteDirect(P1_REGISTER, upperNibble | state.m_buttons);
	}
	else
	{
		memory.WriteDirect(P1_REGISTER, upperNibble | state.m_dPad);
	}
}

void Joypad::ToggleGroup(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue, void* userData)
{
	/*
	uint8_t adjustedVal = prevValue ^ newValue;
	uint8_t currentSelection = ((newValue >> 5) & 0x3);
	if (currentSelection != 3)
	{
		adjustedVal = newValue;
	}
	memory->WriteDirect(P1_REGISTER, adjustedVal);
	*/
}
