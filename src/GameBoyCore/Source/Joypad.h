#pragma once
#include "Memory.h"
#include "../Include/Emulator.h"

class Joypad
{
public:
	Joypad();
	void Init(Memory& memory);
	void Update(EmulatorInputs::InputState state, Memory& memory);
};

