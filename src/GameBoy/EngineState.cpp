#include "EngineState.h"
#include "FileParser.h"
#include "Logging.h"
#include "BackendWin32.h"

UserSettings::UserSettings()
{
	const char* defaultFileName = "usersettings.dat";

	m_filePath = defaultFileName;

	m_isDirty = true;
	Load();
}

void UserSettings::Save()
{
	if (m_isDirty)
	{
		m_isDirty = false;
		FileParser::Write(m_filePath, &m_data, sizeof(m_data));
	}
}

void UserSettings::Load()
{
	std::vector<char> fileBlob;
	if (FileParser::Read(m_filePath, fileBlob))
	{
		memcpy(&m_data, &fileBlob[0], sizeof(m_data));
		m_isDirty = false;
	}
	else
	{
		// Save defaults
		Save();
	}
}

void UserSettings::SetGraphicsScalingFactor(uint32_t factor)
{
	if (m_data.m_graphicsScalingFactor != factor)
	{
		m_data.m_graphicsScalingFactor = factor;
		m_isDirty = true;
	}
}

uint32_t UserSettings::GetGraphicsScalingFactor() const
{
	return m_data.m_graphicsScalingFactor;
}

void UserSettings::SetAudioVolume(float volume)
{
if (m_data.m_audioVolume != volume)
	{
		m_data.m_audioVolume = volume;
		m_isDirty = true;
	}
}

float UserSettings::GetAudioVolume() const
{
	return m_data.m_audioVolume;
}
