#include "EngineState.h"
#include "FileParser.h"
#include "Logging.h"
#include "BackendWin32.h"

const uint32_t INITIAL_STR_BUFFER_SIZE = 2048;
const char LINE_ENDING = '\n';
const char LINE_SEPARATOR = ':';

UserSettings::UserSettings() 
	: m_types()
	, m_filePath()
	, m_graphicsScalingFactor(&m_types, "Graphics.ScalingFactor", 3)
	, m_audioVolume(&m_types, "Audio.MasterVolume", 1.0f)

{
	const char* defaultFileName = "usersettings.ini";

	m_filePath = defaultFileName;

	Load();
}

void UserSettings::Save()
{
	m_types.Apply();
	if (m_types.IsDirty())
	{	
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

void RegisteredTypes::RegisterType(IConfigurableValue* type)
{
	m_types[FileParser::Crc32(type->GetName())] = type;
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
		std::vector<std::string> parts;
		FileParser::SplitString(line, parts, LINE_SEPARATOR);
		if (parts.size() == 2)
		{
			uint32_t typeHash = FileParser::Crc32(parts[0]);
			IConfigurableValue* type = m_types[typeHash];
			type->FromString(parts[1]);
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

inline IConfigurableValue* RegisteredTypes::GetType(const std::string& name)
{
	auto it = m_types.find(FileParser::Crc32(name));
	if (it != m_types.end())
	{
		return it->second;
	}
	return nullptr;
}

IConfigurableValue::IConfigurableValue(RegisteredTypes* typeManager, std::string name) : m_name(name), m_isDirty(false)
{
	typeManager->RegisterType(this);
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
