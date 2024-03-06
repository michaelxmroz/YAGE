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

	EngineData() 
		: m_state(State::RUNNING)
		, m_gameLoaded(false)
		, m_bootromPath("")
		, m_gamePath("") 
	{}

	State m_state;
	bool m_gameLoaded;
	std::string m_bootromPath;
	std::string m_gamePath;
};

