#include "Input.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <WinUser.h>

//TODO separate into platform layer

InputHandler::InputHandler() : m_isPaused(false)
{
}

void InputHandler::Update(EmulatorInputs::InputState& state)
{
	UpdateEmulatorInputs(state);

	if (GetKeyState('P') & 0x8000)
	{
		m_isPaused = !m_isPaused;
	}

	m_debugSaveState = GetKeyState('1') & 0x8000;
	m_debugLoadState = GetKeyState('2') & 0x8000;
	m_turbo = GetKeyState(' ') & 0x8000;
}

void InputHandler::UpdateEmulatorInputs(EmulatorInputs::InputState& state)
{
	if (GetKeyState('W') & 0x8000)
	{
		state.SetButtonDown(EmulatorInputs::DPad::Up);
	}
	if (GetKeyState('A') & 0x8000)
	{
		state.SetButtonDown(EmulatorInputs::DPad::Left);
	}
	if (GetKeyState('S') & 0x8000)
	{
		state.SetButtonDown(EmulatorInputs::DPad::Down);
	}
	if (GetKeyState('D') & 0x8000)
	{
		state.SetButtonDown(EmulatorInputs::DPad::Right);
	}

	if (GetKeyState('N') & 0x8000)
	{
		state.SetButtonDown(EmulatorInputs::Buttons::A);
	}
	if (GetKeyState('M') & 0x8000)
	{
		state.SetButtonDown(EmulatorInputs::Buttons::B);
	}
	if (GetKeyState('K') & 0x8000)
	{
		state.SetButtonDown(EmulatorInputs::Buttons::Start);
	}
	if (GetKeyState('L') & 0x8000)
	{
		state.SetButtonDown(EmulatorInputs::Buttons::Select);
	}
}

bool InputHandler::IsPaused()
{
	return m_isPaused;
}
