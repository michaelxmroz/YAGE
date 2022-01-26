#pragma once
#include "Memory.h"
#include "../Include/Emulator.h"

class Joypad
{
public:
	Joypad();
	void Init(Memory& memory);
	void Update(EmulatorInputs::InputState state, Memory& memory);
private:
	static void ToggleGroup(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue);
};

