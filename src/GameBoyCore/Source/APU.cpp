#include "Helpers.h"
#include "APU.h"
#include "Logging.h"
#include "AudioChannel.h"

#define CHANNEL1_MASTER_CONTROL_ON_OFF_BIT 0x00
#define CHANNEL1_SWEEP_REGISTER 0xFF10
#define CHANNEL1_LENGTH_DUTY_REGISTER 0xFF11
#define CHANNEL1_ENVELOPE_REGISTER 0xFF12
#define CHANNEL1_FREQUENCY_LOW_REGISTER 0xFF13
#define CHANNEL1_CONTROL_FREQ_HIGH_REGISTER 0xFF14

#define CHANNEL2_MASTER_CONTROL_ON_OFF_BIT 0x01
#define CHANNEL2_LENGTH_DUTY_REGISTER 0xFF16
#define CHANNEL2_ENVELOPE_REGISTER 0xFF17
#define CHANNEL2_FREQUENCY_LOW_REGISTER 0xFF18
#define CHANNEL2_CONTROL_FREQ_HIGH_REGISTER 0xFF19

#define CHANNEL3_MASTER_CONTROL_ON_OFF_BIT 0x02
#define CHANNEL3_ON_OFF_REGISTER 0xFF1A
#define CHANNEL3_LENGTH_REGISTER 0xFF1B
#define CHANNEL3_VOLUME_REGISTER 0xFF1C
#define CHANNEL3_FREQUENCY_LOW_REGISTER 0xFF1D
#define CHANNEL3_CONTROL_FREQ_HIGH_REGISTER 0xFF1E

#define CHANNEL4_MASTER_CONTROL_ON_OFF_BIT 0x04
#define CHANNEL4_LENGTH_REGISTER 0xFF20
#define CHANNEL4_ENVELOPE_REGISTER 0xFF21
#define CHANNEL4_FREQUENCY_CLOCK_REGISTER 0xFF22
#define CHANNEL4_CONTROL_REGISTER 0xFF23

#define MASTER_VOLUME_REGISTER 0xFF24
#define SOUND_PANNING_REGISTER 0xFF25
#define AUDIO_MASTER_CONTROL_REGISTER 0xFF26

#define DIVIDER_REGISTER 0xFF04

#define MASTER_VOLUME_RIGHT_BITS 0x07
#define MASTER_VOLUME_LEFT_BITS 0x70
#define MASTER_VOLUME_LEFT_SHIFT 4
#define MASTER_VOLUME_MAX 8

#define APU_ON_OFF_BIT 0x80

#define FRAME_SEQUENCER_NO_TICK 9

using namespace AudioProcessors;

typedef AudioChannel<Sweep, PulseFrequency, Length, Envelope, PulseAmplitude> Channel1;
typedef AudioChannel<NoSweep, PulseFrequency, Length, Envelope, PulseAmplitude> Channel2;
typedef AudioChannel<NoSweep, WaveFrequency, Length, NoEnvelope, WaveAmplitude> Channel3;
typedef AudioChannel<NoSweep, NoiseFrequency, Length, Envelope, NoiseAmplitude> Channel4;

namespace APU_Internal
{

	void ResetAudioRegisters(Memory& memory)
	{
		memory.Write(AUDIO_MASTER_CONTROL_REGISTER, 0xF1);
		memory.Write(MASTER_VOLUME_REGISTER, 0x77);
		memory.Write(SOUND_PANNING_REGISTER, 0xF3);

		memory.Write(CHANNEL1_SWEEP_REGISTER, 0x80);
		memory.Write(CHANNEL1_LENGTH_DUTY_REGISTER, 0xBF);
		memory.Write(CHANNEL1_ENVELOPE_REGISTER, 0xF3);
		memory.Write(CHANNEL1_FREQUENCY_LOW_REGISTER, 0xFF);
		memory.Write(CHANNEL1_CONTROL_FREQ_HIGH_REGISTER, 0xBF);

		memory.Write(CHANNEL2_LENGTH_DUTY_REGISTER, 0x3F);
		memory.Write(CHANNEL2_ENVELOPE_REGISTER, 0x00);
		memory.Write(CHANNEL2_FREQUENCY_LOW_REGISTER, 0xFF);
		memory.Write(CHANNEL2_CONTROL_FREQ_HIGH_REGISTER, 0xBF);

		memory.Write(CHANNEL3_ON_OFF_REGISTER, 0x7F);
		memory.Write(CHANNEL3_LENGTH_REGISTER, 0xFF);
		memory.Write(CHANNEL3_VOLUME_REGISTER, 0x9F);
		memory.Write(CHANNEL3_FREQUENCY_LOW_REGISTER, 0xFF);
		memory.Write(CHANNEL3_CONTROL_FREQ_HIGH_REGISTER, 0xBF);

		memory.Write(CHANNEL4_LENGTH_REGISTER, 0xFF);
		memory.Write(CHANNEL4_ENVELOPE_REGISTER, 0x00);
		memory.Write(CHANNEL4_FREQUENCY_CLOCK_REGISTER, 0x00);
		memory.Write(CHANNEL4_CONTROL_REGISTER, 0xBF);
	}

	void UpdateMasterVolume(Memory& memory, Sample& sample)
	{
		uint8_t rawMasterVolume = memory[MASTER_VOLUME_REGISTER];
		uint8_t volRight = rawMasterVolume & MASTER_VOLUME_RIGHT_BITS;
		uint8_t volLeft = (rawMasterVolume & MASTER_VOLUME_LEFT_BITS) >> MASTER_VOLUME_LEFT_SHIFT;
		volRight += 1;
		volLeft += 1;
		float finalVolRight = static_cast<float>(volRight) / MASTER_VOLUME_MAX;
		float finalVolLeft = static_cast<float>(volLeft) / MASTER_VOLUME_MAX;

		sample.m_left *= finalVolLeft;
		sample.m_right *= finalVolRight;
	}
}

APU::APU() :
	m_channels{
		ChannelData(0, 64, 4, 7, 0x3F, CHANNEL1_MASTER_CONTROL_ON_OFF_BIT, CHANNEL1_CONTROL_FREQ_HIGH_REGISTER, CHANNEL1_LENGTH_DUTY_REGISTER, CHANNEL1_ENVELOPE_REGISTER,CHANNEL1_FREQUENCY_LOW_REGISTER, CHANNEL1_SWEEP_REGISTER),
		ChannelData(1, 64, 4, 7, 0x3F, CHANNEL2_MASTER_CONTROL_ON_OFF_BIT, CHANNEL2_CONTROL_FREQ_HIGH_REGISTER, CHANNEL2_LENGTH_DUTY_REGISTER, CHANNEL2_ENVELOPE_REGISTER, CHANNEL2_FREQUENCY_LOW_REGISTER, 0x0),
		ChannelData(2, 256, 2, 31, 0xFF, CHANNEL3_MASTER_CONTROL_ON_OFF_BIT, CHANNEL3_CONTROL_FREQ_HIGH_REGISTER, CHANNEL3_LENGTH_REGISTER, CHANNEL3_VOLUME_REGISTER, CHANNEL3_FREQUENCY_LOW_REGISTER, CHANNEL3_ON_OFF_REGISTER),
		ChannelData(3, 64, 4, 7, 0x3F, CHANNEL4_MASTER_CONTROL_ON_OFF_BIT, CHANNEL4_CONTROL_REGISTER, CHANNEL4_LENGTH_REGISTER, CHANNEL4_ENVELOPE_REGISTER, CHANNEL4_FREQUENCY_CLOCK_REGISTER, 0x0)
	}
	, m_previousFrameSequencerStep(FRAME_SEQUENCER_NO_TICK)
{
	m_externalAudioBuffer.buffer = nullptr;
	m_externalAudioBuffer.size = 0;
	m_externalAudioBuffer.sampleRate = 0;
	m_externalAudioBuffer.currentPosition = 0;
	m_externalAudioBuffer.resampleRate = 0.0f;
	m_externalAudioBuffer.samplesToGenerate = 0.0f;

}

void APU::Init(Memory& memory)
{
	memory.Write(AUDIO_MASTER_CONTROL_REGISTER, 0xF1);

	memory.RegisterCallback(AUDIO_MASTER_CONTROL_REGISTER, CheckForReset, this);
	memory.RegisterCallback(CHANNEL1_LENGTH_DUTY_REGISTER, AdjustTimer, this);
	memory.RegisterCallback(CHANNEL2_LENGTH_DUTY_REGISTER, AdjustTimer, this);
	memory.RegisterCallback(CHANNEL3_LENGTH_REGISTER, AdjustTimer, this);
	memory.RegisterCallback(CHANNEL4_LENGTH_REGISTER, AdjustTimer, this);

	memory.RegisterCallback(CHANNEL1_ENVELOPE_REGISTER, SetChannelsDACActive, this);
	memory.RegisterCallback(CHANNEL2_ENVELOPE_REGISTER, SetChannelsDACActive, this);
	memory.RegisterCallback(CHANNEL4_ENVELOPE_REGISTER, SetChannelsDACActive, this);

	memory.RegisterCallback(CHANNEL1_CONTROL_FREQ_HIGH_REGISTER, IsChannelTriggered, this);
	memory.RegisterCallback(CHANNEL2_CONTROL_FREQ_HIGH_REGISTER, IsChannelTriggered, this);
	memory.RegisterCallback(CHANNEL3_CONTROL_FREQ_HIGH_REGISTER, IsChannelTriggered, this);
	memory.RegisterCallback(CHANNEL4_CONTROL_REGISTER, IsChannelTriggered, this);

	memory.RegisterCallback(CHANNEL3_ON_OFF_REGISTER, SetChannel3DACActive, this);

	APU_Internal::ResetAudioRegisters(memory);

	m_previousFrameSequencerStep = FRAME_SEQUENCER_NO_TICK;
}

void APU::SetExternalAudioBuffer(float* buffer, uint32_t size, uint32_t sampleRate, uint32_t* startOffset)
{
	m_externalAudioBuffer.buffer = buffer;
	m_externalAudioBuffer.size = size;
	m_externalAudioBuffer.sampleRate = sampleRate;
	m_externalAudioBuffer.currentPosition = startOffset;

	m_HPFLeft.SetParams(100.0f, static_cast<float>(sampleRate));
	m_HPFRight.SetParams(100.0f, static_cast<float>(sampleRate));

	m_externalAudioBuffer.resampleRate = static_cast<double>(m_externalAudioBuffer.sampleRate) / CPU_FREQUENCY;
}

uint32_t APU::Update(Memory& memory, const uint32_t& cyclesPassed)
{
	uint32_t cyclesToStep = cyclesPassed * MCYCLES_TO_CYCLES;

	m_externalAudioBuffer.samplesToGenerate += (static_cast<double>(cyclesToStep) * m_externalAudioBuffer.resampleRate);
	uint32_t samplesgenerated = static_cast<uint32_t>(m_externalAudioBuffer.samplesToGenerate);
	
	Sample sample;
	if ((memory[AUDIO_MASTER_CONTROL_REGISTER] & APU_ON_OFF_BIT) == 0)
	{
		GenerateSamples(m_externalAudioBuffer, sample, m_HPFLeft, m_HPFRight);
		return samplesgenerated;
	}

	uint8_t frameSequencerStep = (memory[DIVIDER_REGISTER] / 32) % 8;
	uint32_t frameSequencerStepTmp = frameSequencerStep;
	frameSequencerStep = frameSequencerStep != m_previousFrameSequencerStep ? frameSequencerStep : FRAME_SEQUENCER_NO_TICK;
	m_previousFrameSequencerStep = frameSequencerStepTmp;
	
	Channel1::Synthesize(m_channels[0], memory, cyclesToStep, frameSequencerStep, sample);
	Channel2::Synthesize(m_channels[1], memory, cyclesToStep, frameSequencerStep, sample);
	Channel3::Synthesize(m_channels[2], memory, cyclesToStep, frameSequencerStep, sample);
	Channel4::Synthesize(m_channels[3], memory, cyclesToStep, frameSequencerStep, sample);

	//TODO is this necessary when we have a HPF?
	if (sample.m_activeChannels != 0)
	{
		sample.m_left /= sample.m_activeChannels;
		sample.m_right /= sample.m_activeChannels;
	}
	
	APU_Internal::UpdateMasterVolume(memory, sample);

	GenerateSamples(m_externalAudioBuffer, sample, m_HPFLeft, m_HPFRight);

	return samplesgenerated;
}

void APU::GenerateSamples(ExternalAudioBuffer& externalAudioBuffer, const Sample& sample, HighPassFilter& hpfLeft, HighPassFilter& hpfRight)
{
	while (externalAudioBuffer.samplesToGenerate >= 1.0f)
	{
		float processedLeft = hpfLeft.ProcessSample(sample.m_left);
		float processedRight = hpfRight.ProcessSample(sample.m_right);
		WriteToAudioBuffer(&externalAudioBuffer, processedLeft, processedRight);

		if (*externalAudioBuffer.currentPosition >= externalAudioBuffer.size)
		{
			*externalAudioBuffer.currentPosition = 0;
		}

		externalAudioBuffer.samplesToGenerate--;
	}
}

void APU::WriteToAudioBuffer(ExternalAudioBuffer* buffer, float leftSample, float rightSample)
{
	float* left = buffer->buffer + (*buffer->currentPosition)++;
	float* right = buffer->buffer + (*buffer->currentPosition)++;
	*left = leftSample;
	*right = rightSample;
}

void APU::CheckForReset(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue, void* userData)
{
	if ((newValue & APU_ON_OFF_BIT) != 0 && (prevValue & APU_ON_OFF_BIT) == 0)
	{
		APU_Internal::ResetAudioRegisters(*memory);
		APU* apu = static_cast<APU*>(userData);

		for (uint32_t i = 0; i < 4; ++i)
		{
			apu->m_channels[i].m_enabled = false;
		}
	}
}

void APU::AdjustTimer(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue, void* userData)
{
	APU* apu = static_cast<APU*>(userData);

	for (uint32_t i = 0; i < CHANNEL_COUNT; ++i)
	{
		ChannelData& channel = apu->m_channels[i];
		if (channel.m_timerRegister == addr)
		{
			channel.m_lengthCounter = channel.m_initialLength - (newValue & channel.m_lengthTimerBits);
		}
	}
}

void APU::SetChannelsDACActive(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue, void* userData)
{
	const uint8_t DAC_ENABLED_BITS = 0xF8;
	APU* apu = static_cast<APU*>(userData);

	for (uint32_t i = 0; i < CHANNEL_COUNT; ++i)
	{
		ChannelData& channel = apu->m_channels[i];
		if (channel.m_volumeEnvelopeRegister != addr)
		{
			continue;
		}

		channel.m_DACEnabled = (newValue & DAC_ENABLED_BITS) != 0;
		if (!channel.m_DACEnabled)
		{
			SetChannelActive(*memory, channel, false);
		}
		break;
	}
}

void APU::SetChannel3DACActive(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue, void* userData)
{
	APU* apu = static_cast<APU*>(userData);
	ChannelData& channel = apu->m_channels[2];
	channel.m_DACEnabled = newValue != 0;
	if (!channel.m_DACEnabled)
	{
		SetChannelActive(*memory, channel, false);
	}
}

void APU::IsChannelTriggered(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue, void* userData)
{
	const uint8_t CONTROL_REGISTER_TRIGGER_BIT = 0x80;
	APU* apu = static_cast<APU*>(userData);

	for (uint32_t i = 0; i < CHANNEL_COUNT; ++i)
	{
		ChannelData& channel = apu->m_channels[i];
		if (channel.m_controlRegister != addr)
		{
			continue;
		}
		channel.m_triggered = (newValue & CONTROL_REGISTER_TRIGGER_BIT) != 0;
		break;
	}
}

APU::HighPassFilter::HighPassFilter() :
	m_alpha(0.0f)
	, m_prevOutput(0.0f)
	, m_prevInput(0.0f)
{
}

void APU::HighPassFilter::SetParams(float cutoff, float sampleRate)
{
	m_alpha = 1.0f / (1.0f + 2.0f * M_PI * cutoff / sampleRate);
}

float APU::HighPassFilter::ProcessSample(float inputSample)
{
	float output = m_alpha * (m_prevOutput + inputSample - m_prevInput);

	m_prevInput = inputSample;
	m_prevOutput = output;
	return output;
}
