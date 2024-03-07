#include "Audio.h"


void Audio::Init()
{
	m_backend.Init();
}

void Audio::Terminate()
{
	m_backend.Terminate();
}

void Audio::Pause()
{
	m_backend.Pause();
}

void Audio::Play()
{
	m_backend.Play();
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
