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

struct InputState
{
	uint8_t m_dPad;
	uint8_t m_buttons;
};

typedef InputState (*JoypadFunc)();

class Joypad
{
public:
	Joypad();
	void Init(Memory& memory);
	void SetInputFunction(JoypadFunc func);
	void Update(Memory& memory);
private:
	InputState m_state;
	JoypadFunc m_queryInput;
};

