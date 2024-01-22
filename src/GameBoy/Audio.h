#pragma once
#include <cstdint>

#define WIN32_BACKEND 0
#define PORTAUDIO_BACKEND 1

#if WIN32_BACKEND
#include "AudioWin32.h"

typedef AudioWin32 AudioBackend;
#elif PORTAUDIO_BACKEND
#include "AudioPortaudio.h"

typedef AudioPortaudio AudioBackend;

#endif

class Audio
{
public:
	void Init();
	void Terminate();

	void Play();

	float* GetAudioBuffer();
	uint32_t GetAudioBufferSize();
	uint32_t GetSampleRate();
	uint32_t* GetWritePosition();

	uint32_t GetFramesConsumed();
	void ResetFramesConsumed();
private:
	AudioBackend m_backend;
};

