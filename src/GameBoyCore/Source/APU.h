#pragma once
#include "Memory.h"

#define CHANNEL_COUNT 4

class APU
{
public:
	APU();
	void Init(Memory& memory);
	void SetExternalAudioBuffer(float* buffer, uint32_t size, uint32_t sampleRate, uint32_t* startOffset);

	uint32_t Update(Memory& memory, const uint32_t& cyclesPassed);

	struct ExternalAudioBuffer
	{
		float* buffer;
		uint32_t size;
		uint32_t sampleRate;
		uint32_t* currentPosition;
		double resampleRate;
		double samplesToGenerate;
	};

private:

	struct Channel
	{
		Channel(uint32_t channelId, uint32_t initialLength, uint32_t frequencyFactor, uint8_t maxSampleLength, uint8_t masterControlOnOffBit, uint16_t controlRegister, uint16_t timerRegister, uint16_t volumeEnvelopeRegister) :
			  m_channelId(channelId)
			, m_masterControlOnOffBit(masterControlOnOffBit)
			, m_controlRegister(controlRegister)
			, m_timerRegister(timerRegister)
			, m_volumeEnvelopeRegister(volumeEnvelopeRegister)
			, m_DACEnabled(true)
			, m_initialLength(initialLength)
			, m_frequencyFactor(frequencyFactor)
			, m_maxSampleLength(maxSampleLength)
			, m_sampleBuffer(0)
			, m_triggered(false)
		{
		};

		const uint8_t m_channelId;
		const uint8_t m_masterControlOnOffBit;
		const uint16_t m_controlRegister;
		const uint16_t m_timerRegister;
		const uint16_t m_volumeEnvelopeRegister;
		const uint32_t m_initialLength;
		const uint32_t m_frequencyFactor;
		const uint8_t m_maxSampleLength;


		uint32_t m_frequencyTimer;
		uint32_t m_dutyStep;
		uint32_t m_periodTimer;
		uint32_t m_currentVolume;
		uint32_t m_lengthCounter;

		//Channel 1 only
		uint32_t m_sweepTimer;
		uint32_t m_shadowFrequency;
		bool m_sweepEnabled;

		//Channel 3 only
		uint8_t m_sampleBuffer;

		//Channel 4 only
		uint16_t m_lfsr;

		bool m_DACEnabled;
		bool m_enabled;
		bool m_triggered;
	};

	static void GenerateSamples(ExternalAudioBuffer& externalAudioBuffer, float left, float right);

	static bool CheckForTrigger(Memory& memory, Channel& channel);
	static void UpdateSweep(Memory& memory, Channel& channel, uint8_t frameSequencerStep, const uint16_t& sweepRegister, const uint16_t& frequencyLow, const uint16_t& frequencyHigh, bool isTriggered);
	static void UpdateNoiseFrequency(Memory& memory, Channel& channel, const uint32_t& cyclesToStep, bool isTriggered);
	static void UpdateWaveFrequency(Memory& memory, Channel& channel, const uint16_t& frequencyLow, const uint16_t& frequencyHigh, const uint32_t& cyclesToStep, bool isTriggered);
	static void UpdateEnvelope(Memory& memory, Channel& channel, uint8_t frameSequencerStep, const uint16_t& envelopeRegister, bool isTriggered);
	static void UpdateLength(Memory& memory, Channel& channel, uint8_t frameSequencerStep, const uint16_t& lengthRegister, const uint16_t& lengthEnableRegister, uint8_t lengthTimerBits, bool isTriggered);
	static float CalculatePulseAmplitude(const Memory& memory,const Channel& channel, const uint16_t& dutyRegister);
	static float CalculateWaveAmplitude(const Memory& memory, const Channel& activeChannel);
	static float CalculateNoiseAmplitude(const Memory& memory, const Channel& channel);

	static void SetChannelActive(Memory& memory, Channel& channel, bool active);
	static void CheckForReset(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue, void* userData);
	static void AdjustTimer(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue, void* userData);
	static void SetChannelsDACActive(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue, void* userData);
	static void SetChannel3DACActive(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue, void* userData);
	static void IsChannelTriggered(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue, void* userData);

	static bool IsWaveChannel(Channel& channel);

	Channel m_channels[CHANNEL_COUNT];
	ExternalAudioBuffer m_externalAudioBuffer;
	uint32_t m_previousFrameSequencerStep;
};