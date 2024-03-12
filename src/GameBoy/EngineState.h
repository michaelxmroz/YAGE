#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <functional>
#include "FileParser.h"
#include "logging.h"
#include "logger.h"

class RegisteredTypes;

//Overengineerd types for user settings to facilitate UI <--> Engine communication through callbacks and serialization/deserialization
class IConfigurableValue
{
public:
	IConfigurableValue(RegisteredTypes* typeManager,std::string name);
	IConfigurableValue(std::string name);

	IConfigurableValue(const IConfigurableValue& other)
		: m_name(other.m_name)
		, m_isDirty(other.m_isDirty)
		, m_isRegistered(other.m_isRegistered)
	{
#if _DEBUG
		if (m_isRegistered)
		{
			LOG_ERROR(string_format("ConfigurableValue: %s trying to copy construct registered value. Please use the non-registering constructor instead and register them later.", m_name.c_str()).c_str());
			assert(false);
		}
#endif
	}

	IConfigurableValue& operator=(const IConfigurableValue& other) = delete;

	virtual ~IConfigurableValue() {}
	bool IsDirty() const { return m_isDirty; }
	void ResetDirtyFlag() { m_isDirty = false; }
	const std::string& GetName() const { return m_name; }
	virtual void DiscardChanges() = 0;
	virtual void Apply() = 0;
	virtual std::string ToString() const = 0;
	virtual void FromString(const std::string& value) = 0;
protected:
	friend class RegisteredTypes;

	std::string m_name;
	bool m_isDirty;
	bool m_isRegistered;
};

class RegisteredTypes
{
public:
	RegisteredTypes() : m_types() 
	{
	}

	void RegisterType(IConfigurableValue* type);
	void DeregisterType(IConfigurableValue* type);

	bool IsDirty() const;
	void ResetDirtyFlag();

    void Apply();
	std::string Save() const;
	void Load(const std::string& data);

	void DiscardChanges();

	IConfigurableValue* GetType(const std::string& name);

private:

	IConfigurableValue* GetValueByKey(uint32_t key);
	bool GetIndex(uint32_t key, uint32_t& index);
	std::vector<std::pair<uint32_t, IConfigurableValue*>> m_types;
};

template <typename T>
class ConfigurableValue : public IConfigurableValue
{
public:
	ConfigurableValue(RegisteredTypes* typeManager, const char* name, T value)
		: IConfigurableValue(typeManager, name)
		, m_value(value) 
		, m_oldValue(value)
	{
	}

	ConfigurableValue(const char* name, T value)
		: IConfigurableValue(name)
		, m_value(value)
		, m_oldValue(value)
	{
	}

	~ConfigurableValue() override {}

	ConfigurableValue(const ConfigurableValue& other) 
		: IConfigurableValue(other)
		, m_value(other.m_value)
		, m_oldValue(other.m_oldValue)
		, m_callback(other.m_callback)
	{
	}

	ConfigurableValue& operator=(const ConfigurableValue& other)
	{
		if (this != &other)
		{
#if _DEBUG
			if (m_isRegistered)
			{
				LOG_ERROR(string_format("ConfigurableValue: %s trying to assign registered value. Please use the non-registering constructor instead and register them later.", m_name.c_str()).c_str());
				assert(false);
			}
#endif
			m_name = other.m_name;
			m_isDirty = other.m_isDirty;
			m_isRegistered = other.m_isRegistered;
			m_value = other.m_value;
			m_oldValue = other.m_oldValue;
			m_callback = other.m_callback;
		}
		return *this;
	}

	void RegisterCallback(std::function<void(T)> callback) 
	{
		m_callback = callback;
	}

	T GetValue() const 
	{
#if _DEBUG
		if (!m_isRegistered)
		{
			LOG_ERROR(string_format("ConfigurableValue: %s is not registered", m_name.c_str()).c_str());
			assert(false);
		}
#endif
		return m_value; 
	}

	void SetValue(const T& value) 
	{ 
#if _DEBUG
		if (!m_isRegistered)
		{
			LOG_ERROR(string_format("ConfigurableValue: %s is not registered", m_name.c_str()).c_str());
			assert(false);
		}
#endif
		m_value = value; 
	}

	void Apply() override
	{ 
		if(m_value != m_oldValue)
		{
			m_oldValue = m_value;
			m_isDirty = true;
			if(m_callback)
			{
				m_callback(m_value);
			}
		}
		else
		{
			m_isDirty = false;
		}
	}

	void DiscardChanges() override { m_value = m_oldValue; }

	std::string ToString() const override
	{
		return FileParser::ToString(m_value);
	}

	void FromString(const std::string& value) override
	{
		m_value = FileParser::FromString<T>(value);
		m_oldValue = m_value;
		m_isDirty = false;
	}

private:
	T m_value;
	T m_oldValue;
	std::function<void(T)> m_callback;
};

class UserSettings
{
private:
	RegisteredTypes m_types;
public:
	UserSettings();

	void Save();
	void Load();
	void DiscardChanges();

	IConfigurableValue* GetType(const std::string& name)
	{
		return m_types.GetType(name);
	}

	void AddRecentFile(const std::string& file);

	ConfigurableValue<bool> m_systemUseBootrom;
	ConfigurableValue<std::string> m_systemBootromPath;
	ConfigurableValue<uint32_t> m_graphicsScalingFactor;
	ConfigurableValue<float> m_audioVolume;
	std::vector<ConfigurableValue<std::string>> m_recentFiles;
	uint32_t m_recentFilesIndex;

private:
	std::string m_filePath;

	void ReorderRecentFiles();
};

class StateMachine
{
public:
	enum class EngineState : uint8_t
	{
		RUNNING = 0,
		EXIT,
		RESET,
		PAUSED,
		COUNT
	};

	StateMachine();

	void SetState(EngineState state);
	EngineState GetState() const;

	void RegisterStateChangeCallback(EngineState previousState, EngineState newState, std::function<void()> callback);

private:
	EngineState m_state;
	std::vector<std::vector<std::vector<std::function<void()>>>> m_stateChangeCallbacks;
};

struct EngineData
{
	enum class SaveLoadState : uint8_t
	{
		NONE = 0,
		SAVE = 1,
		LOAD = 2
	};

	EngineData() 
		: m_engineState()
		, m_saveLoadState(SaveLoadState::NONE)
		, m_userSettings()
		, m_gameLoaded(false)
		, m_bootromPath("")
		, m_gamePath("") 
		, m_saveLoadPath("")
	{}

	StateMachine m_engineState;
	SaveLoadState m_saveLoadState;
	UserSettings m_userSettings;
	bool m_gameLoaded;
	std::string m_bootromPath;
	std::string m_gamePath;
	std::string m_saveLoadPath;
private:
    EngineData(const EngineData&) = delete;
	EngineData& operator=(const EngineData&) = delete;
};

