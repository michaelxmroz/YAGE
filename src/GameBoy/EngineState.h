#pragma once
#include <cstdint>
#include <string>

struct EngineData
{
	enum class State : uint8_t
	{
		RUNNING = 0,
		EXIT = 1,
		RESET = 2
	};

	enum class SaveLoadState : uint8_t
	{
		NONE = 0,
		SAVE = 1,
		LOAD = 2
	};

	EngineData() 
		: m_state(State::RUNNING)
		, m_saveLoadState(SaveLoadState::NONE)
		, m_gameLoaded(false)
		, m_bootromPath("")
		, m_gamePath("") 
		, m_saveLoadPath("")
	{}

	State m_state;
	SaveLoadState m_saveLoadState;
	bool m_gameLoaded;
	std::string m_bootromPath;
	std::string m_gamePath;
	std::string m_saveLoadPath;
};

