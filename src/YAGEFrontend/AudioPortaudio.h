#pragma once
#include "portaudio.h"
#include <cstdint>

class AudioPortaudio
{
public:
	void Init();
	void Terminate();
	void Pause();
	void Play();

	void SetVolume(float volume);

	float* GetAudioBuffer();
	uint32_t GetAudioBufferSize();
	uint32_t GetSampleRate();
	uint32_t* GetWritePosition();

	uint32_t GetFramesConsumed();
	void ResetFramesConsumed();
private:
	float* m_buffer;
	uint32_t m_writePosition;
	uint32_t m_playbackPosition;
	PaStream* m_stream;

	float m_volume;

	uint32_t m_framesConsumed;

	void ErrorHandler(PaError err);

    static int paCallback(const void* inputBuffer, void* outputBuffer,
        unsigned long framesPerBuffer,
        const PaStreamCallbackTimeInfo* timeInfo,
        PaStreamCallbackFlags statusFlags,
        void* userData);

};

