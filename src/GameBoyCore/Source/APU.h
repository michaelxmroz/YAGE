#pragma once
#include "Memory.h"
#include "AudioChannel.h"

#define CHANNEL_COUNT 4

class APU : ISerializable
{
public:
	APU(Serializer* serializer);
	void Init(Memory& memory);
	void SetExternalAudioBuffer(float* buffer, uint32_t size, uint32_t sampleRate, uint32_t* startOffset);

	uint32_t Update(Memory& memory, uint32_t cyclesPassed, float turboSpeed);

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
	static void CheckForSweepReverse(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue, void* userData);
	static void AdjustTimer(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue, void* userData);
	static void SetChannelsDACActive(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue, void* userData);
	static void SetChannel3DACActive(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue, void* userData);
	static void IsChannelTriggered(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue, void* userData);

	ChannelData m_channels[CHANNEL_COUNT];
	ExternalAudioBuffer m_externalAudioBuffer;
	uint8_t m_cachedFrameSequencerPulse;
	uint32_t m_frameSequencerStep;
	bool m_wasDivBit4Set;
	HighPassFilter m_HPFLeft;
	HighPassFilter m_HPFRight;

	uint32_t m_accumulatedCycles;
	uint32_t m_totalSequencerSteps = 0;

	void UpdateFrameSequencer(Memory& memory);

	// Inherited via ISerializable
	void Serialize(std::vector<Chunk>& chunks, std::vector<uint8_t>& data) override;
	void Deserialize(const Chunk* chunks, const uint32_t& chunkCount, const uint8_t* data, const uint32_t& dataSize) override;
};