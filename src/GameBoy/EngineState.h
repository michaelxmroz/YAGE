#pragma once
#include <cstdint>
#include <string>


class UserSettings
{
public:
	UserSettings();

	void Save();
	void Load();

	void SetGraphicsScalingFactor(uint32_t factor);	
	uint32_t GetGraphicsScalingFactor() const;

	void SetAudioVolume(float volume);
	float GetAudioVolume() const;
private:

	struct UserData
	{
		uint32_t m_graphicsScalingFactor = 3;
		float m_audioVolume = 0.5f;
	} m_data;

	bool m_isDirty;
	std::string m_filePath;
};

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
		, m_userSettings()
		, m_gameLoaded(false)
		, m_bootromPath("")
		, m_gamePath("") 
		, m_saveLoadPath("")
	{}

	State m_state;
	SaveLoadState m_saveLoadState;
	UserSettings m_userSettings;
	bool m_gameLoaded;
	std::string m_bootromPath;
	std::string m_gamePath;
	std::string m_saveLoadPath;
};

