#include "Audio.h"


void Audio::Init()
{
	m_backend.Init();
	m_state = AudioState::STOPPED;
}

void Audio::Terminate()
{
	m_backend.Terminate();
}

void Audio::Pause()
{
	if(m_state != AudioState::STOPPED)
	{
		m_state = AudioState::STOPPED;
		m_backend.Pause();
	}
}

void Audio::Play()
{
	if (m_state != AudioState::PLAYING)
	{
		m_backend.Play();
		m_state = AudioState::PLAYING;
	}
}

void Audio::SetVolume(float volume)
{
	m_backend.SetVolume(volume);
}

void Audio::RegisterOptionsCallbacks(UserSettings& userSettings)
{
	userSettings.m_audioVolume.RegisterCallback(std::bind(&Audio::SetVolume, this, std::placeholders::_2));

	SetVolume(userSettings.m_audioVolume.GetValue());
}

void Audio::RegisterEngineStateChangeCallbacks(StateMachine& stateMachine)
{
	stateMachine.RegisterStateChangeCallback(StateMachine::EngineState::RUNNING, StateMachine::EngineState::PAUSED, std::bind(&Audio::Pause, this));
	stateMachine.RegisterStateChangeCallback(StateMachine::EngineState::PAUSED, StateMachine::EngineState::RUNNING, std::bind(&Audio::Play, this));
}

float* Audio::GetAudioBuffer()
{
	return m_backend.GetAudioBuffer();
}

uint32_t Audio::GetAudioBufferSize()
{
	return m_backend.GetAudioBufferSize();
}

uint32_t Audio::GetSampleRate()
{
	return m_backend.GetSampleRate();
}

uint32_t* Audio::GetWritePosition()
{
	return m_backend.GetWritePosition();
}

uint32_t Audio::GetFramesConsumed()
{
	return m_backend.GetFramesConsumed();
}

void Audio::ResetFramesConsumed()
{
	return m_backend.ResetFramesConsumed();
}
