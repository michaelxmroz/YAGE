#pragma once
#include "Memory.h"

#define CHANNEL_COUNT 4

class APU
{
public:
	APU();
	void Init(Memory& memory);
	void SetExternalAudioBuffer(float* buffer, uint32_t size, uint32_t sampleRate, uint32_t* startOffset);

	void Update(Memory& memory, const uint32_t& cyclesPassed);
private:

	struct ExternalAudioBuffer
	{
		float* buffer;
		uint32_t size;
		uint32_t sampleRate;
		uint32_t* currentPosition;
		float resampleRate;
	};

	struct Channel
	{
		uint32_t m_frequencyTimer;
		uint32_t m_wavePosition;
		uint32_t m_periodTimer;
		uint32_t m_currentVolume;
		uint32_t m_lengthCounter;
		bool m_enabled;
	};

	static bool CheckForTrigger(Memory& memory, Channel& channel, const uint16_t& triggerRegister);
	static void UpdateTimer(Memory& memory, Channel& channel, const uint16_t& frequencyLow, const uint16_t& frequencyHigh, const uint32_t& cyclesToStep, bool isTriggered);
	static void UpdateEnvelope(Memory& memory, Channel& channel, uint8_t frameSequencerStep, const uint16_t& envelopeRegister, bool isTriggered);
	static void UpdateLength(Memory& memory, Channel& channel, uint8_t frameSequencerStep, const uint16_t& lengthRegister, const uint16_t& lengthEnableRegister, bool isTriggered, bool isWave);
	static float CalculateWaveAmplitude(Memory& memory, Channel& channel, const uint16_t& dutyRegister);
	static void Pan(Memory& memory, Channel& channel, float amplitude, uint8_t panRegisterOffset, const uint16_t& panRegister, float& mixedAmplitudeLeft, float& mixedAmplitudeRight);

	Channel m_channels[CHANNEL_COUNT];
	ExternalAudioBuffer m_externalAudioBuffer;
};

