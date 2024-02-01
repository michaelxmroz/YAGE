#pragma once
#include <cstdint>
#include <type_traits>
#include "Memory.h"

struct Sample
{
	Sample() :
		  m_left (0.0f)
		, m_right(0.0f)
		, m_activeChannels(0)
	{}
	float m_left;
	float m_right;
	uint32_t m_activeChannels;
};

struct ChannelData
{
	ChannelData(uint32_t channelId,
		uint32_t initialLength,
		uint32_t frequencyFactor,
		uint8_t maxSampleLength,
		uint8_t lengthTimerBits,
		uint8_t masterControlOnOffBit,
		uint16_t controlRegister,
		uint16_t timerRegister,
		uint16_t volumeEnvelopeRegister,
		uint16_t frequencyRegister,
		uint16_t sweepRegister) :
		m_channelId(channelId)
		, m_masterControlOnOffBit(masterControlOnOffBit)
		, m_controlRegister(controlRegister)
		, m_timerRegister(timerRegister)
		, m_volumeEnvelopeRegister(volumeEnvelopeRegister)
		, m_sweepRegister(sweepRegister)
		, m_frequencyRegister(frequencyRegister)
		, m_DACEnabled(true)
		, m_initialLength(initialLength)
		, m_frequencyFactor(frequencyFactor)
		, m_maxSampleLength(maxSampleLength)
		, m_sampleBuffer(0)
		, m_triggered(false)
		, m_lengthTimerBits(lengthTimerBits)
	{
	};

	const uint8_t m_channelId;
	const uint8_t m_masterControlOnOffBit;
	const uint16_t m_sweepRegister; //NRx0
	const uint16_t m_timerRegister; //NRx1
	const uint16_t m_volumeEnvelopeRegister; //NRx2
	const uint16_t m_frequencyRegister; //NRx3
	const uint16_t m_controlRegister; //NRx4

	const uint32_t m_initialLength;
	const uint32_t m_frequencyFactor;
	const uint8_t m_maxSampleLength;
	const uint8_t m_lengthTimerBits;

	uint32_t m_frequencyTimer;
	uint32_t m_dutyStep;
	uint32_t m_periodTimer;
	uint32_t m_envelopePeriod;
	bool m_envelopeIncrease;
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

namespace AudioProcessors
{
	void SetChannelActive(Memory& memory, ChannelData& channel, bool active);

	namespace AudioChannel_Internal
	{
		float DAC(uint8_t amplitude);
		void Pan(Memory& memory, float amplitude, uint8_t panRegisterOffset, float& mixedAmplitudeLeft, float& mixedAmplitudeRight);
		uint32_t GetFrequency(const Memory& memory, const uint16_t& frequencyHighRegister, const uint16_t& frequencyLowRegister);
		void SetFrequency(Memory& memory, const uint16_t& frequencyHighRegister, const uint16_t& frequencyLowRegister, uint32_t frequency);
		bool CheckForTrigger(Memory& memory, ChannelData& channel);
	}

	class Sweep
	{
	public:
		static void UpdateSweep(Memory& memory, ChannelData& channel, uint8_t frameSequencerStep, bool isTriggered);
	private:
		const static uint32_t SWEEP_FREQUENCY_MAX_VALUE = 2047;

		static bool ReloadSweepTimer(const Memory& memory, uint32_t& sweepTimer, uint16_t sweepRegister, uint8_t sweepPeriodBits, uint8_t sweepPeriodOffset);
		static uint32_t CalculateNewSweepFrequency(bool& channelActive, uint32_t shadowFrequency, uint8_t sweepShift, bool isDecreasing);
	};


	class NoSweep
	{
	public:
		static void UpdateSweep(Memory& /*memory*/, ChannelData& /*channel*/, uint8_t /*frameSequencerStep*/, bool /*isTriggered*/)
		{
		}
	};

	class PulseFrequency
	{
	public:

		static void UpdateFrequency(Memory& memory, ChannelData& channel, uint32_t cyclesToStep, bool isTriggered);
		static bool UpdateFrequencyInternal(Memory& memory, ChannelData& channel, uint32_t cyclesToStep, bool isTriggered, uint32_t initialDutyStep);
	};

	class WaveFrequency
	{
	public:
		static void UpdateFrequency(Memory& memory, ChannelData& channel, const uint32_t& cyclesToStep, bool isTriggered);
	};

	class NoiseFrequency
	{
	public:
		static void UpdateFrequency(Memory& memory, ChannelData& channel, const uint32_t& cyclesToStep, bool isTriggered);
	private:
		static uint32_t GetNoiseFrequencyTimer(uint8_t frequencyRegister);
	};

	class Length
	{
	public:
		static void UpdateLength(Memory& memory, ChannelData& channel, uint8_t frameSequencerStep, bool isTriggered);
	};

	class Envelope
	{
	public:
		static void UpdateVolume(Memory& memory, ChannelData& channel, uint8_t frameSequencerStep, bool isTriggered);
	};

	class NoEnvelope
	{
	public:
		static void UpdateVolume(Memory& /*memory*/, ChannelData& /*channel*/, uint8_t /*frameSequencerStep*/, bool /*isTriggered*/)
		{
		}
	};

	class PulseAmplitude
	{
	public:
		static float UpdateAmplitude(const Memory& memory, const ChannelData& channel);
	};

	class WaveAmplitude
	{
	public:
		static float UpdateAmplitude(const Memory& memory, const ChannelData& channel);
	};

	class NoiseAmplitude
	{
	public:
		static float UpdateAmplitude(const Memory& memory, const ChannelData& channel);
	};

}

template<class SweepProcessor, class FrequencyProcessor, class LengthProcessor, class VolumeProcessor, class AmplitudeProcessor>
class AudioChannel
{
public:
	static void Synthesize(ChannelData& data, Memory& memory, uint32_t cyclesToStep, uint8_t frameSequencerStep, Sample& sampleOut)
	{
		using namespace AudioProcessors;
		bool isTriggered = AudioChannel_Internal::CheckForTrigger(memory, data);
		if (data.m_enabled)
		{
			SweepProcessor::UpdateSweep(memory, data, frameSequencerStep, isTriggered);
			FrequencyProcessor::UpdateFrequency(memory, data, cyclesToStep, isTriggered);
			LengthProcessor::UpdateLength(memory, data, frameSequencerStep, isTriggered);
			if (data.m_enabled)
			{
				VolumeProcessor::UpdateVolume(memory, data, frameSequencerStep, isTriggered);
				float amplitude = AmplitudeProcessor::UpdateAmplitude(memory, data);
				AudioChannel_Internal::Pan(memory, amplitude, data.m_channelId, sampleOut.m_left, sampleOut.m_right);
				sampleOut.m_activeChannels++;
			}
		}
	}
};


