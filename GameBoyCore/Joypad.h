#pragma once
#include "Memory.h"

enum class DPad
{
	Right = 1,
	Left = 2,
	Up = 4,
	Down = 8
};

enum class Buttons
{
	A = 1,
	B = 2,
	Select = 4,
	Start = 8
};

class Joypad
{
public:
	void Init(Memory& memory);
	void Update(Memory& memory);
private:
	uint8_t m_dPad;
	uint8_t m_Buttons;
};

