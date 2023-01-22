#pragma once
#include "Emulator.h"

class InputHandler
{
public:
	InputHandler();

	void Update(EmulatorInputs::InputState& state);
	bool IsPaused();

	bool m_debugSaveState;
	bool m_debugLoadState;
	bool m_turbo;

private:
	void UpdateEmulatorInputs(EmulatorInputs::InputState& state);

	bool m_isPaused;
};