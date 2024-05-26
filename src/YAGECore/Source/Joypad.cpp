#include "Joypad.h"
#include "Helpers.h"
#include "Interrupts.h"

#define P1_REGISTER 0xFF00

bool IsActionGroupSelected(uint8_t upperNibble)
{
	return (upperNibble & 0x20) == 0;
}

bool IsDPadGroupSelected(uint8_t upperNibble)
{
	return (upperNibble & 0x10) == 0;
}

Joypad::Joypad()
{
}

void Joypad::Init(Memory& memory)
{

	memory.AddIOUnusedBitsOverride(P1_REGISTER, 0b11000000);
	memory.AddIOReadOnlyBitsOverride(P1_REGISTER, 0b00001111);

	memory.Write(P1_REGISTER, 0xCF);

	memory.RegisterCallback(P1_REGISTER, CheckForInterrupt, this);
}

void Joypad::Update(EmulatorInputs::InputState state, Memory& memory)
{
	uint8_t upperNibble = memory[P1_REGISTER] & 0xF0;
	if (IsActionGroupSelected(upperNibble))
	{
		memory.WriteIO(P1_REGISTER, upperNibble | state.m_buttons);
	}
	else if(IsDPadGroupSelected(upperNibble))
	{
		memory.WriteIO(P1_REGISTER, upperNibble | state.m_dPad);
	}
	else
	{
		memory.WriteIO(P1_REGISTER, upperNibble | 0xF);
	}
}

void Joypad::CheckForInterrupt(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue, void* userData)
{
	uint8_t lowerNibbleOld = prevValue & 0xF;
	uint8_t lowerNibbleNew = newValue & 0xF;
	if((lowerNibbleOld & ~lowerNibbleNew) != 0)
	{
		Interrupts::RequestInterrupt(Interrupts::Types::Joypad, *memory);
	}
}
