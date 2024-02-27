#pragma once
#include "Memory.h"
#include "AudioChannel.h"

#define CHANNEL_COUNT 4

#define M_PI 3.14159265f

class APU
{
public:
	APU();
	void Init(Memory& memory);
	void SetExternalAudioBuffer(float* buffer, uint32_t size, uint32_t sampleRate, uint32_t* startOffset);

	uint32_t Update(Memory& memory, const uint32_t& cyclesPassed);

private:

	class HighPassFilter
	{
	public:
		HighPassFilter();

		void SetParams(float cutoff, float sampleRate);
		float ProcessSample(float inputSample);

	private:
		float m_alpha;
		float m_prevOutput;
		float m_prevInput;
	};

	struct ExternalAudioBuffer
	{
		float* buffer;
		uint32_t size;
		uint32_t sampleRate;
		uint32_t* currentPosition;
		double resampleRate;
		double samplesToGenerate;
	};

	static void GenerateSamples(ExternalAudioBuffer& externalAudioBuffer, const Sample& sample, HighPassFilter& hpfLeft, HighPassFilter& hpfRight);
	static void WriteToAudioBuffer(APU::ExternalAudioBuffer* buffer, float leftSample, float rightSample);

	static void CheckForReset(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue, void* userData);
	static void AdjustTimer(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue, void* userData);
	static void SetChannelsDACActive(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue, void* userData);
	static void SetChannel3DACActive(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue, void* userData);
	static void IsChannelTriggered(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue, void* userData);

	ChannelData m_channels[CHANNEL_COUNT];
	ExternalAudioBuffer m_externalAudioBuffer;
	uint32_t m_previousFrameSequencerStep;
	HighPassFilter m_HPFLeft;
	HighPassFilter m_HPFRight;
};