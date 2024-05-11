#include "Input.h"
#include "Backend.h"


namespace InputHandler_Internal
{

	InputActions RawInputToAction(uint32_t rawInput, const std::unordered_map<uint32_t, InputActions>& inputMap)
	{
		auto it = inputMap.find(rawInput);
		if (it != inputMap.end())
		{
			return it->second;
		}
		return InputActions::Count;
	}
}

InputHandler::InputHandler()
{
	Backend::GetDefaultInputMapping(m_inputMap);
	m_buttonStates[InputActions::Pause] = ButtonState();
	m_buttonStates[InputActions::QuickSave] = ButtonState();
	m_buttonStates[InputActions::QuickLoad] = ButtonState();
	m_buttonStates[InputActions::Turbo] = ButtonState();
}

void InputHandler::RegisterOptionsCallbacks(UserSettings& userSettings)
{
	bool setDefaults = false;
	for(uint32_t i = 0; i < userSettings.m_keyBindings.size(); ++i)
	{
		auto& inputOption = userSettings.m_keyBindings[i];
		InputActions action = static_cast<InputActions>(i);

		if (inputOption.GetValue() == 0) //Set defaults
		{
			for(auto& input : m_inputMap)
			{
				if (input.second == action)
				{
					inputOption.SetValue(input.first);
					setDefaults = true;
					break;
				}
			}
		}

		auto callback = [this, action](uint32_t oldValue, uint32_t newValue)
			{
				auto it = m_inputMap.find(oldValue);
				if (it != m_inputMap.end())
				{
					m_inputMap.erase(it);
				}
				m_inputMap[newValue] = action;

			};


		inputOption.RegisterCallback(callback);
	}

	if (setDefaults)
	{
		userSettings.Save();
	}
	else
	{
		m_inputMap.clear();
		for (uint32_t i = 0; i < userSettings.m_keyBindings.size(); ++i)
		{
			auto& inputOption = userSettings.m_keyBindings[i];
			InputActions action = static_cast<InputActions>(i);
			m_inputMap[inputOption.GetValue()] = action;
		}
	}
}

void InputHandler::Update(EngineData& data, const std::unordered_map<uint32_t, bool>& rawInputEvents, EmulatorInputs::InputState& state)
{
	for (auto& rawInputs : rawInputEvents)
	{
		InputActions action = InputHandler_Internal::RawInputToAction(rawInputs.first, m_inputMap);
		bool isPressed = rawInputs.second;
		UpdateButtonStates(action, isPressed);

		if (isPressed)
		{
			UpdateEmulatorInputs(state, action);
		}
	}

	for(auto& buttonState : m_buttonStates)
	{
		if (buttonState.second.IsJustPressed())
		{
			switch (buttonState.first)
			{
			case InputActions::Pause:
				data.m_engineState.SetState(data.m_engineState.GetState() == StateMachine::EngineState::RUNNING ? StateMachine::EngineState::PAUSED : StateMachine::EngineState::RUNNING);
				break;
			case InputActions::QuickSave:
				data.m_saveLoadState = EngineData::SaveLoadState::SAVE;
				break;
			case InputActions::QuickLoad:
				data.m_saveLoadState = EngineData::SaveLoadState::LOAD;
				break;
			case InputActions::Turbo:
				data.m_turbo = !data.m_turbo;
				break;
			}
		}
	}
}


void InputHandler::UpdateEmulatorInputs(EmulatorInputs::InputState& state, InputActions action)
{
	switch (action)
	{
	case InputActions::Up:
		state.SetButtonDown(EmulatorInputs::DPad::Up);
		break;
	case InputActions::Down:
		state.SetButtonDown(EmulatorInputs::DPad::Down);
		break;
	case InputActions::Left:
		state.SetButtonDown(EmulatorInputs::DPad::Left);
		break;
	case InputActions::Right:
		state.SetButtonDown(EmulatorInputs::DPad::Right);
		break;
	case InputActions::A:
		state.SetButtonDown(EmulatorInputs::Buttons::A);
		break;
	case InputActions::B:
		state.SetButtonDown(EmulatorInputs::Buttons::B);
		break;
	case InputActions::Start:
		state.SetButtonDown(EmulatorInputs::Buttons::Start);
		break;
	case InputActions::Select:
		state.SetButtonDown(EmulatorInputs::Buttons::Select);
		break;
	}
}

void InputHandler::UpdateButtonStates(InputActions input, bool isPressed)
{
	auto it = m_buttonStates.find(input);
	if (it != m_buttonStates.end())
	{
		m_buttonStates[input].Update(isPressed);
	}
}
