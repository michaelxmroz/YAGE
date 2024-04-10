#include "Helpers.h"
#include "APU.h"
#include "Logging.h"
#include "AudioChannel.h"

#define APU_REGISTERS_BEGIN 0xFF10
#define APU_REGISTERS_END 0xFF2F
#define APU_WAVE_RAM_BEGIN 0xFF30
#define APU_WAVE_RAM_END 0xFF3F

#define CHANNEL1_MASTER_CONTROL_ON_OFF_BIT 0x01
#define CHANNEL1_SWEEP_REGISTER 0xFF10
#define CHANNEL1_LENGTH_DUTY_REGISTER 0xFF11
#define CHANNEL1_ENVELOPE_REGISTER 0xFF12
#define CHANNEL1_FREQUENCY_LOW_REGISTER 0xFF13
#define CHANNEL1_CONTROL_FREQ_HIGH_REGISTER 0xFF14

#define CHANNEL2_MASTER_CONTROL_ON_OFF_BIT 0x02
#define CHANNEL2_UNUSED_REGISTER 0xFF15
#define CHANNEL2_LENGTH_DUTY_REGISTER 0xFF16
#define CHANNEL2_ENVELOPE_REGISTER 0xFF17
#define CHANNEL2_FREQUENCY_LOW_REGISTER 0xFF18
#define CHANNEL2_CONTROL_FREQ_HIGH_REGISTER 0xFF19

#define CHANNEL3_MASTER_CONTROL_ON_OFF_BIT 0x04
#define CHANNEL3_ON_OFF_REGISTER 0xFF1A
#define CHANNEL3_LENGTH_REGISTER 0xFF1B
#define CHANNEL3_VOLUME_REGISTER 0xFF1C
#define CHANNEL3_FREQUENCY_LOW_REGISTER 0xFF1D
#define CHANNEL3_CONTROL_FREQ_HIGH_REGISTER 0xFF1E

#define CHANNEL4_MASTER_CONTROL_ON_OFF_BIT 0x08
#define CHANNEL4_UNUSED_REGISTER 0xFF1F
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

#define FRAME_SEQUENCER_NO_PULSE 0

using namespace AudioProcessors;

typedef AudioChannel<Sweep, PulseFrequency, Length, Envelope, PulseAmplitude> Channel1;
typedef AudioChannel<NoSweep, PulseFrequency, Length, Envelope, PulseAmplitude> Channel2;
typedef AudioChannel<NoSweep, WaveFrequency, Length, NoEnvelope, WaveAmplitude> Channel3;
typedef AudioChannel<NoSweep, NoiseFrequency, Length, Envelope, NoiseAmplitude> Channel4;

namespace APU_Internal
{
	void TriggerChannel(Memory& memory, ChannelData& data, uint32_t index)
	{
		switch (index)
		{
		case 0:
			Channel1::Trigger(memory, data);
			break;
		case 1:
			Channel2::Trigger(memory, data);
			break;
		case 2:
			Channel3::Trigger(memory, data);
			break;
		case 3:
			Channel4::Trigger(memory, data);
			break;
		}
	}

	void SetupRegisterBitsOverrides(Memory& memory)
	{
		memory.AddIOUnusedBitsOverride(CHANNEL1_SWEEP_REGISTER, 0b10000000);
		memory.AddIOUnusedBitsOverride(CHANNEL1_CONTROL_FREQ_HIGH_REGISTER, 0b00111000);

		memory.AddIOUnusedBitsOverride(CHANNEL2_UNUSED_REGISTER, 0b11111111);
		memory.AddIOUnusedBitsOverride(CHANNEL2_CONTROL_FREQ_HIGH_REGISTER, 0b00111000);

		memory.AddIOUnusedBitsOverride(CHANNEL3_ON_OFF_REGISTER, 0b01111111);
		memory.AddIOUnusedBitsOverride(CHANNEL3_VOLUME_REGISTER, 0b10011111);
		memory.AddIOUnusedBitsOverride(CHANNEL3_CONTROL_FREQ_HIGH_REGISTER, 0b00111000);

		memory.AddIOUnusedBitsOverride(CHANNEL4_UNUSED_REGISTER, 0b11111111);
		memory.AddIOUnusedBitsOverride(CHANNEL4_LENGTH_REGISTER, 0b11000000);
		memory.AddIOUnusedBitsOverride(CHANNEL4_CONTROL_REGISTER, 0b00111111);

		memory.AddIOUnusedBitsOverride(AUDIO_MASTER_CONTROL_REGISTER, 0b01110000);

		memory.AddIOWriteOnlyBitsOverride(CHANNEL1_LENGTH_DUTY_REGISTER, 0b00111111);
		memory.AddIOWriteOnlyBitsOverride(CHANNEL1_FREQUENCY_LOW_REGISTER, 0b11111111);
		memory.AddIOWriteOnlyBitsOverride(CHANNEL1_CONTROL_FREQ_HIGH_REGISTER, 0b10000111);

		memory.AddIOWriteOnlyBitsOverride(CHANNEL2_LENGTH_DUTY_REGISTER, 0b00111111);
		memory.AddIOWriteOnlyBitsOverride(CHANNEL2_FREQUENCY_LOW_REGISTER, 0b11111111);
		memory.AddIOWriteOnlyBitsOverride(CHANNEL2_CONTROL_FREQ_HIGH_REGISTER, 0b10000111);

		memory.AddIOWriteOnlyBitsOverride(CHANNEL3_LENGTH_REGISTER, 0b11111111);
		memory.AddIOWriteOnlyBitsOverride(CHANNEL3_FREQUENCY_LOW_REGISTER, 0b11111111);
		memory.AddIOWriteOnlyBitsOverride(CHANNEL3_CONTROL_FREQ_HIGH_REGISTER, 0b10000111);

		memory.AddIOWriteOnlyBitsOverride(CHANNEL4_LENGTH_REGISTER, 0b00111111);
		memory.AddIOWriteOnlyBitsOverride(CHANNEL4_CONTROL_REGISTER, 0b10000000);

		memory.AddIOWriteOnlyRange(APU_WAVE_RAM_BEGIN, APU_WAVE_RAM_END);


		memory.AddIOReadOnlyBitsOverride(AUDIO_MASTER_CONTROL_REGISTER, 0b00001111);
	}

	void ResetAudioRegistersToBootValues(Memory& memory)
	{
		memory.WriteIO(AUDIO_MASTER_CONTROL_REGISTER, 0xF1);
		memory.WriteIO(MASTER_VOLUME_REGISTER, 0x77);
		memory.WriteIO(SOUND_PANNING_REGISTER, 0xF3);

		memory.WriteIO(CHANNEL1_SWEEP_REGISTER, 0x80);
		memory.WriteIO(CHANNEL1_LENGTH_DUTY_REGISTER, 0xBF);
		memory.WriteIO(CHANNEL1_ENVELOPE_REGISTER, 0xF3);
		memory.WriteIO(CHANNEL1_FREQUENCY_LOW_REGISTER, 0xFF);
		memory.WriteIO(CHANNEL1_CONTROL_FREQ_HIGH_REGISTER, 0xBF);

		memory.WriteIO(CHANNEL2_LENGTH_DUTY_REGISTER, 0x3F);
		memory.WriteIO(CHANNEL2_ENVELOPE_REGISTER, 0x00);
		memory.WriteIO(CHANNEL2_FREQUENCY_LOW_REGISTER, 0xFF);
		memory.WriteIO(CHANNEL2_CONTROL_FREQ_HIGH_REGISTER, 0xBF);

		memory.WriteIO(CHANNEL3_ON_OFF_REGISTER, 0x7F);
		memory.WriteIO(CHANNEL3_LENGTH_REGISTER, 0xFF);
		memory.WriteIO(CHANNEL3_VOLUME_REGISTER, 0x9F);
		memory.WriteIO(CHANNEL3_FREQUENCY_LOW_REGISTER, 0xFF);
		memory.WriteIO(CHANNEL3_CONTROL_FREQ_HIGH_REGISTER, 0xBF);

		memory.WriteIO(CHANNEL4_LENGTH_REGISTER, 0xFF);
		memory.WriteIO(CHANNEL4_ENVELOPE_REGISTER, 0x00);
		memory.WriteIO(CHANNEL4_FREQUENCY_CLOCK_REGISTER, 0x00);
		memory.WriteIO(CHANNEL4_CONTROL_REGISTER, 0xBF);
	}

	void UpdateMasterVolume(Memory& memory, Sample& sample)
	{
		uint8_t rawMasterVolume = memory.ReadIO(MASTER_VOLUME_REGISTER);
		uint8_t volRight = rawMasterVolume & MASTER_VOLUME_RIGHT_BITS;
		uint8_t volLeft = (rawMasterVolume & MASTER_VOLUME_LEFT_BITS) >> MASTER_VOLUME_LEFT_SHIFT;
		volRight += 1;
		volLeft += 1;
		float finalVolRight = static_cast<float>(volRight) / MASTER_VOLUME_MAX;
		float finalVolLeft = static_cast<float>(volLeft) / MASTER_VOLUME_MAX;

		sample.m_left *= finalVolLeft;
		sample.m_right *= finalVolRight;
	}

	//Obscure length counter behaviour: if length enable bit is set in the first half of the frame sequencer step,
	// the length counter is decremented
	void CheckForLengthEnableBug(uint32_t frameSequencerStep, const uint8_t& newValue, const uint8_t& prevValue, bool resetLength, ChannelData& channel, bool triggered, Memory* memory)
	{
		const uint8_t CONTROL_REGISTER_LENGTH_ENABLE_BIT = 0x40;
		bool firstHalfSeqencerStep = (frameSequencerStep & 1) != 0;
		bool lengthEnable = (newValue & CONTROL_REGISTER_LENGTH_ENABLE_BIT) != 0;
		bool previousLengthEnable = (prevValue & CONTROL_REGISTER_LENGTH_ENABLE_BIT) != 0;
		bool justEnabled = lengthEnable && (!previousLengthEnable || resetLength);
		if (justEnabled && channel.m_lengthCounter > 0 && firstHalfSeqencerStep)
		{
			channel.m_lengthCounter--;
			if (channel.m_lengthCounter == 0 && !triggered)
			{
				SetChannelActive(*memory, channel, false);
			}
			else if (channel.m_lengthCounter == 0 && triggered)
			{
				channel.m_lengthCounter = channel.m_initialLength - 1;
			}
		}
	}

	void InitWaveRamToDefaults(Memory& memory)
	{
		const uint32_t waveRamSize = APU_WAVE_RAM_END - APU_WAVE_RAM_BEGIN + 1;
		const uint8_t defaults[waveRamSize] = { 0x84, 0x40, 0x43, 0xAA, 0x2D, 0x78, 0x92, 0x3C, 0x60, 0x59, 0x59, 0xB0, 0x34, 0xB8, 0x2E, 0xDA };

		for (uint16_t i = 0; i < waveRamSize; ++i)
		{

			memory.WriteIO(APU_WAVE_RAM_BEGIN + i, defaults[i]);
		}
	}
}

APU::APU(Serializer* serializer) : ISerializable(serializer)
	, m_channels{
		ChannelData(0, 64, 4, 7, 0x3F, CHANNEL1_MASTER_CONTROL_ON_OFF_BIT, CHANNEL1_CONTROL_FREQ_HIGH_REGISTER, CHANNEL1_LENGTH_DUTY_REGISTER, CHANNEL1_ENVELOPE_REGISTER,CHANNEL1_FREQUENCY_LOW_REGISTER, CHANNEL1_SWEEP_REGISTER),
		ChannelData(1, 64, 4, 7, 0x3F, CHANNEL2_MASTER_CONTROL_ON_OFF_BIT, CHANNEL2_CONTROL_FREQ_HIGH_REGISTER, CHANNEL2_LENGTH_DUTY_REGISTER, CHANNEL2_ENVELOPE_REGISTER, CHANNEL2_FREQUENCY_LOW_REGISTER, 0x0),
		ChannelData(2, 256, 2, 31, 0xFF, CHANNEL3_MASTER_CONTROL_ON_OFF_BIT, CHANNEL3_CONTROL_FREQ_HIGH_REGISTER, CHANNEL3_LENGTH_REGISTER, CHANNEL3_VOLUME_REGISTER, CHANNEL3_FREQUENCY_LOW_REGISTER, CHANNEL3_ON_OFF_REGISTER),
		ChannelData(3, 64, 4, 7, 0x3F, CHANNEL4_MASTER_CONTROL_ON_OFF_BIT, CHANNEL4_CONTROL_REGISTER, CHANNEL4_LENGTH_REGISTER, CHANNEL4_ENVELOPE_REGISTER, CHANNEL4_FREQUENCY_CLOCK_REGISTER, 0x0)
	}
	, m_frameSequencerStep(0)
	, m_wasDivBit4Set(false)
	, m_cachedFrameSequencerPulse(FRAME_SEQUENCER_NO_PULSE)
{
	m_externalAudioBuffer.buffer = nullptr;
	m_externalAudioBuffer.size = 0;
	m_externalAudioBuffer.sampleRate = 0;
	m_externalAudioBuffer.currentPosition = 0;
	m_externalAudioBuffer.resampleRate = 0.0f;
	m_externalAudioBuffer.samplesToGenerate = 0.0f;

	m_totalSequencerSteps = 0;
}

void APU::Init(Memory& memory)
{
	APU_Internal::SetupRegisterBitsOverrides(memory);

	memory.RegisterCallback(AUDIO_MASTER_CONTROL_REGISTER, CheckForReset, this);

	memory.RegisterCallback(CHANNEL1_SWEEP_REGISTER, CheckForSweepReverse, this);

	memory.RegisterCallback(CHANNEL1_LENGTH_DUTY_REGISTER, AdjustTimer, this);
	memory.RegisterCallback(CHANNEL2_LENGTH_DUTY_REGISTER, AdjustTimer, this);
	memory.RegisterCallback(CHANNEL3_LENGTH_REGISTER, AdjustTimer, this);
	memory.RegisterCallback(CHANNEL4_LENGTH_REGISTER, AdjustTimer, this);

	memory.RegisterCallback(CHANNEL1_ENVELOPE_REGISTER, SetChannelsDACActive, this);
	memory.RegisterCallback(CHANNEL2_ENVELOPE_REGISTER, SetChannelsDACActive, this);
	memory.RegisterCallback(CHANNEL4_ENVELOPE_REGISTER, SetChannelsDACActive, this);

	APU_Internal::ResetAudioRegistersToBootValues(memory);

	APU_Internal::InitWaveRamToDefaults(memory);

	memory.RegisterCallback(CHANNEL1_CONTROL_FREQ_HIGH_REGISTER, IsChannelTriggered, this);
	memory.RegisterCallback(CHANNEL2_CONTROL_FREQ_HIGH_REGISTER, IsChannelTriggered, this);
	memory.RegisterCallback(CHANNEL3_CONTROL_FREQ_HIGH_REGISTER, IsChannelTriggered, this);
	memory.RegisterCallback(CHANNEL4_CONTROL_REGISTER, IsChannelTriggered, this);

	memory.RegisterCallback(CHANNEL3_ON_OFF_REGISTER, SetChannel3DACActive, this);

	m_accumulatedCycles = 0;
	m_frameSequencerStep = 0;
	m_cachedFrameSequencerPulse = FRAME_SEQUENCER_NO_PULSE;
	m_wasDivBit4Set = false;
}

void APU::SetExternalAudioBuffer(float* buffer, uint32_t size, uint32_t sampleRate, uint32_t* startOffset)
{
	m_externalAudioBuffer.buffer = buffer;
	m_externalAudioBuffer.size = size;
	m_externalAudioBuffer.sampleRate = sampleRate;
	m_externalAudioBuffer.currentPosition = startOffset;

	m_HPFLeft.SetParams(1000.0f, static_cast<float>(sampleRate));
	m_HPFRight.SetParams(1000.0f, static_cast<float>(sampleRate));

	m_externalAudioBuffer.resampleRate = static_cast<double>(m_externalAudioBuffer.sampleRate) / CPU_FREQUENCY;
}

uint32_t APU::Update(Memory& memory, uint32_t cyclesPassed, float turboSpeed)
{
	uint32_t cyclesToStep = cyclesPassed * MCYCLES_TO_CYCLES;

	m_externalAudioBuffer.samplesToGenerate += (static_cast<double>(cyclesToStep) * m_externalAudioBuffer.resampleRate) * (1.0 / static_cast<double>(turboSpeed));
	
	m_accumulatedCycles += cyclesToStep;
	
	uint32_t samplesgenerated = static_cast<uint32_t>(m_externalAudioBuffer.samplesToGenerate);

	UpdateFrameSequencer(memory);

	const uint8_t LENGTH_TICK_RATE = 1;
	const uint8_t SWEEP_TICK_RATE = 3;
	const uint8_t VOLUME_TICK_RATE = 7;

	if ((m_cachedFrameSequencerPulse & LENGTH_TICK_RATE) == LENGTH_TICK_RATE)
	{
		Channel1::UpdateLength(memory, m_channels[0]);
		Channel2::UpdateLength(memory, m_channels[1]);
		Channel3::UpdateLength(memory, m_channels[2]);
		Channel4::UpdateLength(memory, m_channels[3]);
	}
	if((m_cachedFrameSequencerPulse & SWEEP_TICK_RATE) == SWEEP_TICK_RATE)
	{
		Channel1::UpdateSweep(memory, m_channels[0]);
		Channel2::UpdateSweep(memory, m_channels[1]);
		Channel3::UpdateSweep(memory, m_channels[2]);
		Channel4::UpdateSweep(memory, m_channels[3]);
	}
	if ((m_cachedFrameSequencerPulse & VOLUME_TICK_RATE) == VOLUME_TICK_RATE)
	{
		Channel1::UpdateVolume(memory, m_channels[0]);
		Channel2::UpdateVolume(memory, m_channels[1]);
		Channel3::UpdateVolume(memory, m_channels[2]);
		Channel4::UpdateVolume(memory, m_channels[3]);
	}

	m_cachedFrameSequencerPulse = FRAME_SEQUENCER_NO_PULSE;

	if (m_externalAudioBuffer.samplesToGenerate < 1.0f )
	{
		return samplesgenerated;
	}

	Sample sample;
	if ((memory.ReadIO(AUDIO_MASTER_CONTROL_REGISTER) & APU_ON_OFF_BIT) == 0)
	{
		m_accumulatedCycles = 0;
		GenerateSamples(m_externalAudioBuffer, sample, m_HPFLeft, m_HPFRight);
		return samplesgenerated;
	}

	Channel1::Render(memory, m_channels[0], m_accumulatedCycles, sample);
	Channel2::Render(memory, m_channels[1], m_accumulatedCycles, sample);
	Channel3::Render(memory, m_channels[2], m_accumulatedCycles, sample);
	Channel4::Render(memory, m_channels[3], m_accumulatedCycles, sample);

	m_accumulatedCycles = 0;

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

void APU::UpdateFrameSequencer(Memory& memory)
{
	bool divBit4 = (memory.ReadIO(DIVIDER_REGISTER) & 0x10) != 0;
	if (!divBit4 && m_wasDivBit4Set)
	{
		m_totalSequencerSteps++;
		m_frameSequencerStep = (m_frameSequencerStep + 1) % 8;
		m_cachedFrameSequencerPulse = m_frameSequencerStep;
	}
	m_wasDivBit4Set = divBit4;
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
	if ((prevValue & APU_ON_OFF_BIT) != 0 && (newValue & APU_ON_OFF_BIT) == 0)
	{
		APU* apu = static_cast<APU*>(userData);

		for (uint32_t i = 0; i < 4; ++i)
		{
			AudioProcessors::SetChannelActive(*memory, apu->m_channels[i], false);
		}

		memory->ClearRange(APU_REGISTERS_BEGIN, APU_REGISTERS_END);
		memory->AddIOReadOnlyRange(APU_REGISTERS_BEGIN, APU_REGISTERS_END);
		//Length registers are not affected by the APU being turned off on DMG
		memory->AddIOReadOnlyBitsOverride(CHANNEL1_LENGTH_DUTY_REGISTER, 0b11000000);
		memory->AddIOReadOnlyBitsOverride(CHANNEL2_LENGTH_DUTY_REGISTER, 0b11000000);
		memory->AddIOReadOnlyBitsOverride(CHANNEL3_LENGTH_REGISTER, 0b00000000);
		memory->AddIOReadOnlyBitsOverride(CHANNEL4_LENGTH_REGISTER, 0b00000000);
		memory->AddIOReadOnlyBitsOverride(AUDIO_MASTER_CONTROL_REGISTER, 0b00001111);
	}
	else if ((prevValue & APU_ON_OFF_BIT) == 0 && (newValue & APU_ON_OFF_BIT) != 0)
	{
		APU* apu = static_cast<APU*>(userData);

		apu->m_frameSequencerStep = 0;
		memory->RemoveIOReadOnlyRange(APU_REGISTERS_BEGIN, APU_REGISTERS_END);
		memory->AddIOReadOnlyBitsOverride(AUDIO_MASTER_CONTROL_REGISTER, 0b00001111);
	}
}

//Obscure APU behaviour: If sweep direction is reversed after at least a single sweep update, the channel is disabled
void APU::CheckForSweepReverse(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue, void* userData)
{
	const uint8_t SWEEP_DIRECTION_BIT = 0x08;

	APU* apu = static_cast<APU*>(userData);

	ChannelData& channel = apu->m_channels[0];

	if(prevValue & SWEEP_DIRECTION_BIT && !(newValue & SWEEP_DIRECTION_BIT) && channel.m_decreasingFrequencyCalculationPerformed)
	{
		SetChannelActive(*memory, channel, false);
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

		bool triggered = (newValue & CONTROL_REGISTER_TRIGGER_BIT) != 0;

		bool resetLength = false;
		if (triggered && channel.m_lengthCounter == 0)
		{
			resetLength = true;
		}

		if (triggered)
		{
			if (channel.m_DACEnabled)
			{
				SetChannelActive(*memory, channel, true);
			}

			APU_Internal::TriggerChannel(*memory, apu->m_channels[i], i);
		}	

		APU_Internal::CheckForLengthEnableBug(apu->m_frameSequencerStep, newValue, prevValue, resetLength, channel, triggered, memory);
		
		break;
	}
}

void APU::Serialize(std::vector<Chunk>& chunks, std::vector<uint8_t>& data)
{
	uint32_t dataSize = sizeof(ChannelData) * CHANNEL_COUNT + sizeof(HighPassFilter) * 2 + sizeof(uint32_t)
		+ sizeof(bool) + sizeof(uint8_t) + sizeof(uint32_t);
	uint8_t* rawData = CreateChunkAndGetDataPtr(chunks, data, dataSize, ChunkId::APU);

	for (uint32_t i = 0; i < CHANNEL_COUNT; ++i)
	{
		WriteAndMove(rawData, m_channels + i, sizeof(ChannelData));
	}

	WriteAndMove(rawData, &m_HPFLeft, sizeof(HighPassFilter));
	WriteAndMove(rawData, &m_HPFRight, sizeof(HighPassFilter));
	WriteAndMove(rawData, &m_frameSequencerStep, sizeof(uint32_t));
	WriteAndMove(rawData, &m_wasDivBit4Set, sizeof(bool));
	WriteAndMove(rawData, &m_cachedFrameSequencerPulse, sizeof(uint8_t));
	WriteAndMove(rawData, &m_accumulatedCycles, sizeof(uint32_t));
}

void APU::Deserialize(const Chunk* chunks, const uint32_t& chunkCount, const uint8_t* data, const uint32_t& dataSize)
{
	const Chunk* myChunk = FindChunk(chunks, chunkCount, ChunkId::APU);
	if (myChunk == nullptr)
	{
		return;
	}

	data += myChunk->m_offset;

	for (uint32_t i = 0; i < CHANNEL_COUNT; ++i)
	{
		ReadAndMove(data, m_channels + i, sizeof(ChannelData));
	}

	ReadAndMove(data, &m_HPFLeft, sizeof(HighPassFilter));
	ReadAndMove(data, &m_HPFRight, sizeof(HighPassFilter));
	ReadAndMove(data, &m_frameSequencerStep, sizeof(uint32_t));
	ReadAndMove(data, &m_wasDivBit4Set, sizeof(bool));
	ReadAndMove(data, &m_cachedFrameSequencerPulse, sizeof(uint8_t));
	ReadAndMove(data, &m_accumulatedCycles, sizeof(uint32_t));
}

APU::HighPassFilter::HighPassFilter() :
	m_alpha(0.0f)
	, m_prevOutput(0.0f)
	, m_prevInput(0.0f)
{
}

void APU::HighPassFilter::SetParams(float cutoff, float sampleRate)
{
	const float M_PI = 3.141592654f;
	m_alpha = 1.0f / (1.0f + 2.0f * M_PI * cutoff / sampleRate);
}

float APU::HighPassFilter::ProcessSample(float inputSample)
{
	float output = m_alpha * (m_prevOutput + inputSample - m_prevInput);

	m_prevInput = inputSample;
	m_prevOutput = output;
	return output;
}
