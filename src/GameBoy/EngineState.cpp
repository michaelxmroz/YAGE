#include "EngineState.h"
#include "FileParser.h"
#include "Logging.h"
#include "BackendWin32.h"
#include "Input.h"

const uint32_t INITIAL_STR_BUFFER_SIZE = 2048;
const char LINE_ENDING = '\n';
const char LINE_SEPARATOR = ':';
const uint32_t MAX_RECENT_FILES = 5;

UserSettings::UserSettings()
	: m_types()
	, m_filePath()
	, m_systemUseBootrom(&m_types, "System.UseBootrom", false)
	, m_systemBootromPath(&m_types, "System.BootromPath", "")
	, m_graphicsScalingFactor(&m_types, "Graphics.ScalingFactor", 3)
	, m_audioVolume(&m_types, "Audio.MasterVolume", 1.0f)
	, m_recentFilesIndex(0)

{
	m_recentFiles.reserve(MAX_RECENT_FILES);
	for(uint32_t i = 0; i < MAX_RECENT_FILES; ++i)
	{
		std::string name = "Files.Recent" + std::to_string(i);
		m_recentFiles.push_back(ConfigurableValue<std::string>(name.c_str(), ""));
		m_types.RegisterType(&m_recentFiles[i]);
	}

	m_keyBindings.reserve(static_cast<uint32_t>(InputActions::Count));
	for(uint32_t i = 0; i < static_cast<uint32_t>(InputActions::Count); ++i)
	{
		std::string name = "Input.KeyBindings." + std::string(InputActionNames[i]);
		m_keyBindings.push_back(ConfigurableValue<uint32_t>(name.c_str(), 0));
		m_types.RegisterType(&m_keyBindings[i]);
	}

	const char* defaultFileName = "usersettings.ini";

	m_filePath = defaultFileName;

	Load();
}

void UserSettings::Save()
{
	m_types.Apply();
	if (m_types.IsDirty())
	{	
		ReorderRecentFiles();

		std::string serializedData = m_types.Save();
		FileParser::Write(m_filePath, serializedData.c_str(), serializedData.length());
		m_types.ResetDirtyFlag();
	}
}

void UserSettings::Load()
{
	std::string loadBuffer;
	if (FileParser::Read(m_filePath, loadBuffer))
	{
		m_types.Load(loadBuffer);
	}
	else
	{
		// Save defaults
		Save();
	}
}

void UserSettings::DiscardChanges()
{
	m_types.DiscardChanges();
}

void UserSettings::AddRecentFile(const std::string& file)
{
	m_recentFiles[m_recentFilesIndex].SetValue(file);
	m_recentFilesIndex = (m_recentFilesIndex + 1) % MAX_RECENT_FILES;
}

void UserSettings::ReorderRecentFiles()
{
	std::vector<std::string> tmp;
	for (auto& file : m_recentFiles)
	{
		if (!file.GetValue().empty())
		{
			tmp.push_back(file.GetValue());
		}
	}
	std::rotate(tmp.begin(), tmp.begin() + m_recentFilesIndex, tmp.end());
	
	for(uint32_t i = 0; i < MAX_RECENT_FILES; ++i)
	{
		if (i < tmp.size())
		{
			m_recentFiles[i].SetValue(tmp[i]);
		}
		else
		{
			m_recentFiles[i].SetValue("");
		}
	}
}

void RegisteredTypes::RegisterType(IConfigurableValue* type)
{
	type->m_isRegistered = true;
	uint32_t key = FileParser::Crc32(type->GetName());
	if (GetValueByKey(FileParser::Crc32(type->GetName())) == nullptr)
	{
		m_types.push_back({ key,type });
	}
}

void RegisteredTypes::DeregisterType(IConfigurableValue* type)
{
	uint32_t key = FileParser::Crc32(type->GetName());
	uint32_t index = 0;
	if (GetIndex(key, index))
	{
		type->m_isRegistered = false;
		m_types.erase(m_types.begin() + index);
	}
}

bool RegisteredTypes::IsDirty() const
{
	for (auto& type : m_types)
	{
		if (type.second->IsDirty())
		{
			return true;
		}
	}
	return false;
}

void RegisteredTypes::ResetDirtyFlag()
{
	for (auto& type : m_types)
	{
		type.second->ResetDirtyFlag();
	}
}

void RegisteredTypes::Apply()
{
	for (auto& type : m_types)
	{
		type.second->Apply();
	}
}

std::string RegisteredTypes::Save() const
{
	std::string result;
	result.reserve(INITIAL_STR_BUFFER_SIZE);

	for (auto& type : m_types)
	{
		result += type.second->GetName() + LINE_SEPARATOR + type.second->ToString() + LINE_ENDING;
	}
	return result;
}

void RegisteredTypes::Load(const std::string& data)
{
    std::vector<std::string> lines;
	FileParser::SplitString(data, lines, LINE_ENDING);

	for (auto& line : lines)
	{
		std::string name;
		std::string value;
		FileParser::SplitStringOnce(line, name, value, LINE_SEPARATOR);
		if (!name.empty() && !value.empty())
		{
			uint32_t typeHash = FileParser::Crc32(name);
			IConfigurableValue* type = GetValueByKey(typeHash);
			if (type != nullptr)
			{
				type->FromString(value);
			}
		}
	}
}

void RegisteredTypes::DiscardChanges()
{
for (auto& type : m_types)
	{
		type.second->DiscardChanges();
	}
}

IConfigurableValue* RegisteredTypes::GetType(const std::string& name)
{
	return GetValueByKey(FileParser::Crc32(name));
}

IConfigurableValue* RegisteredTypes::GetValueByKey(uint32_t key) 
{
	for (const auto& pair : m_types) 
	{
		if (pair.first == key) 
		{
			return pair.second;
		}
	}
	return nullptr;
}

bool RegisteredTypes::GetIndex(uint32_t key, uint32_t& index)
{
	for (uint32_t i = 0; i < m_types.size(); ++i) 
	{
		if (m_types[i].first == key) 
		{
			index = i;
			return true;
		}
	}
	return false;
}

IConfigurableValue::IConfigurableValue(RegisteredTypes* typeManager, std::string name) 
	: m_name(name)
	, m_isDirty(false)
	, m_isRegistered(false)
{
	typeManager->RegisterType(this);
}

IConfigurableValue::IConfigurableValue(std::string name) 
	: m_name(name)
	, m_isDirty(false)
	, m_isRegistered(false)
{
}

StateMachine::StateMachine() : m_state(EngineState::RUNNING)
{
	m_stateChangeCallbacks.resize(static_cast<uint8_t>(EngineState::COUNT));
	for (auto& state : m_stateChangeCallbacks)
	{
		state.resize(static_cast<uint8_t>(EngineState::COUNT));
	}
}

void StateMachine::SetState(EngineState state)
{
	if (m_state != state)
	{
		for (auto& callback : m_stateChangeCallbacks[static_cast<uint8_t>(m_state)][static_cast<uint8_t>(state)])
		{
			callback();
		}
	}
	m_state = state;
}
StateMachine::EngineState StateMachine::GetState() const
{
	return m_state;
}

void StateMachine::RegisterStateChangeCallback(EngineState previousState, EngineState newState, std::function<void()> callback)
{
	m_stateChangeCallbacks[static_cast<uint8_t>(previousState)][static_cast<uint8_t>(newState)].push_back(callback);
}