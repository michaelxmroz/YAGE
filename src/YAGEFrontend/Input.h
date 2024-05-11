#pragma once
#include "Emulator.h"
#include "EngineState.h"

enum class InputActions : uint32_t
{
	Up = 0,
	Down,
	Left,
	Right,
	A,
	B,
	Select,
	Start,
	Pause,
	Turbo,
	QuickSave,
	QuickLoad,
	Count
};

//You must update this array to match the InputActions enum
//Yes this solution is crap, and should be replaced with sth like https://github.com/aantron/better-enums sooner rather than later
static const char* InputActionNames[static_cast<uint32_t>(InputActions::Count)] = {
	"Up",
	"Down",
	"Left",
	"Right",
	"A",
	"B",
	"Select",
	"Start",
	"Pause",
	"Turbo",
	"QuickSave",
	"QuickLoad"
};

class ButtonState
{
public:
	ButtonState() : m_isPressed(false), m_wasPressed(false) {}

	void Update(bool isPressed)
	{
		m_wasPressed = m_isPressed;
		m_isPressed = isPressed;
	}

	bool IsPressed() const { return m_isPressed; }
	bool WasPressed() const { return m_wasPressed; }
	bool IsReleased() const { return !m_isPressed; }
	bool WasReleased() const { return !m_wasPressed; }
	bool IsJustPressed() const { return m_isPressed && !m_wasPressed; }
	bool IsJustReleased() const { return !m_isPressed && m_wasPressed; }
private:
	bool m_isPressed;
	bool m_wasPressed;
};

class InputHandler
{
public:
	InputHandler();

	void RegisterOptionsCallbacks(UserSettings& userSettings);

	void Update(EngineData& stateMachine, const std::unordered_map<uint32_t, bool>& rawInputEvents, EmulatorInputs::InputState& state);

private:
	void UpdateEmulatorInputs(EmulatorInputs::InputState& state, InputActions action);
	void UpdateButtonStates(InputActions input, bool isPressed);

	std::unordered_map<uint32_t, InputActions> m_inputMap;

	std::unordered_map<InputActions, ButtonState> m_buttonStates;
};