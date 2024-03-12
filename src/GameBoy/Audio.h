#pragma once
#include <cstdint>
#include "EngineState.h"

#define WIN32_AUDIO 0
#define PORTAUDIO_BACKEND 1

#if WIN32_AUDIO
#include "AudioWin32.h"

typedef AudioWin32 AudioBackend;
#elif PORTAUDIO_BACKEND
#include "AudioPortaudio.h"

typedef AudioPortaudio AudioBackend;

#endif

class Audio
{
public:
	enum class AudioState : uint32_t
	{
		PLAYING = 0,
		STOPPED
	};

	void Init();
	void Terminate();
	void Pause();
	void Play();

	void SetVolume(float volume);
	void RegisterOptionsCallbacks(UserSettings& userSettings);
	void RegisterEngineStateChangeCallbacks(StateMachine& stateMachine);

	float* GetAudioBuffer();
	uint32_t GetAudioBufferSize();
	uint32_t GetSampleRate();
	uint32_t* GetWritePosition();

	uint32_t GetFramesConsumed();
	void ResetFramesConsumed();

private:
	AudioBackend m_backend;
	AudioState m_state;
};

