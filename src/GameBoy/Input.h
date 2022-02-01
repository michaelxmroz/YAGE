#pragma once
#include "Emulator.h"

class InputHandler
{
public:
	InputHandler();

	void Update(EmulatorInputs::InputState& state);
	bool IsPaused();
private:
	void UpdateEmulatorInputs(EmulatorInputs::InputState& state);

	bool m_isPaused;
};