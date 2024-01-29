#include "Helpers.h"
#include "APU.h"
#include "Logging.h"


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
#define CHANNEL4_POLY_COUNTER_REGISTER 0xFF22
#define CHANNEL4_CONTROL_REGISTER 0xFF23

#define MASTER_VOLUME_REGISTER 0xFF24
#define SOUND_PANNING_REGISTER 0xFF25
#define AUDIO_MASTER_CONTROL_REGISTER 0xFF26

#define WAVE_PATTERN_RAM_BEGIN 0xFF30
#define WAVE_PATTERN_RAM_END 0xFF3F

#define DIVIDER_REGISTER 0xFF04

#define MASTER_VOLUME_RIGHT_BITS 0x07
#define MASTER_VOLUME_LEFT_BITS 0x70
#define MASTER_VOLUME_LEFT_SHIFT 4
#define MASTER_VOLUME_MAX 8

#define SOUND_LENGTH_TICK_RATE 2
#define SOUND_LENGTH_ENABLE_BIT 0x40
#define SOUND_LENGTH_TIMER_BITS 0x3F
#define SOUND_LENGTH_CHANNEL3_BITS 0xFF

#define CHANNEL1_FREQUENCY_SWEEP_TICK_RATE 4
#define SWEEP_PERIOD_DEFAULT_VALUE 8
#define SWEEP_FREQUENCY_MAX_VALUE 2047

#define APU_ON_OFF_BIT 0x80

#define FRAME_SEQUENCER_NO_TICK 9

const uint8_t DUTY_WAVEFORMS[4]
{
	0b00000001,
	0b10000001,
	0b10000111,
	0b01111110,
};

namespace APU_Internal
{
	void WriteToAudioBuffer(APU::ExternalAudioBuffer* buffer, float leftSample, float rightSample)
	{
		float* left = buffer->buffer + (*buffer->currentPosition)++;
		float* right = buffer->buffer + (*buffer->currentPosition)++;
		*left = leftSample;
		*right = rightSample;
	}

	void ResetAudioRegisters(Memory& memory)
	{
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
		memory.Write(CHANNEL4_POLY_COUNTER_REGISTER, 0x00);
		memory.Write(CHANNEL4_CONTROL_REGISTER, 0xBF);
	}


	float DAC(uint8_t amplitude)
	{
		//return -(static_cast<float>(amplitude) / 7.5f - 1.0f);
		return static_cast<float>(amplitude) / 15.0f;
	}

	void Pan(Memory& memory, float amplitude, uint8_t panRegisterOffset, const uint16_t& panRegister, float& mixedAmplitudeLeft, float& mixedAmplitudeRight)
	{
		bool hasRight = ((memory[panRegister] >> panRegisterOffset) & 0x01) > 0;
		bool hasLeft = ((memory[panRegister] >> (panRegisterOffset + 4)) & 0x01) > 0;

		mixedAmplitudeLeft += hasLeft ? amplitude : 0.0f;
		mixedAmplitudeRight += hasRight ? amplitude : 0.0f;
	}

	void UpdateMasterVolume(Memory& memory, float& mixedAmplitudeLeft, float& mixedAmplitudeRight)
	{
		uint8_t rawMasterVolume = memory[MASTER_VOLUME_REGISTER];
		uint8_t volRight = rawMasterVolume & MASTER_VOLUME_RIGHT_BITS;
		uint8_t volLeft = (rawMasterVolume & MASTER_VOLUME_LEFT_BITS) >> MASTER_VOLUME_LEFT_SHIFT;
		volRight += 1;
		volLeft += 1;
		float finalVolRight = static_cast<float>(volRight) / MASTER_VOLUME_MAX;
		float finalVolLeft = static_cast<float>(volLeft) / MASTER_VOLUME_MAX;

		mixedAmplitudeLeft *= finalVolLeft;
		mixedAmplitudeRight *= finalVolRight;
	}

	bool ReloadSweepTimer(Memory& memory, uint32_t& sweepTimer, const uint16_t& sweepRegister, const uint8_t& sweepPeriodBits, const uint8_t& sweepPeriodOffset)
	{
		sweepTimer = (memory[sweepRegister] & sweepPeriodBits) >> sweepPeriodOffset;
		if (sweepTimer == 0)
		{
			sweepTimer = SWEEP_PERIOD_DEFAULT_VALUE;
			return true;
		}
		return false;
	}

	uint32_t CalculateNewSweepFrequency(bool& channelActive, uint32_t shadowFrequency, uint8_t sweepShift, bool isDecreasing)
	{
		uint32_t newFrequency = shadowFrequency >> sweepShift;

		if (isDecreasing) 
		{
			newFrequency = shadowFrequency - newFrequency;
		}
		else 
		{
			newFrequency = shadowFrequency + newFrequency;
		}

		/* overflow check */
		if (newFrequency > SWEEP_FREQUENCY_MAX_VALUE) 
		{
			channelActive = false;
		}

		return newFrequency;
	}

	uint32_t GetFrequency(Memory& memory, const uint16_t& frequencyHighRegister, const uint16_t& frequencyLowRegister)
	{
		const uint8_t FREQUENCY_HIGH_BITS = 0x07;
		return ((static_cast<uint32_t>(memory[frequencyHighRegister]) & FREQUENCY_HIGH_BITS) << 8) | memory[frequencyLowRegister];
	}

	void SetFrequency(Memory& memory, const uint16_t& frequencyHighRegister, const uint16_t& frequencyLowRegister, uint32_t frequency)
	{
		const uint8_t FREQUENCY_HIGH_BITS = 0x07;
		const uint8_t FREQUENCY_LOW_BITS = 0x0F;
		memory.Write(frequencyLowRegister, frequency & FREQUENCY_LOW_BITS);
		uint8_t highRegisterOther = memory[frequencyHighRegister] & ~FREQUENCY_HIGH_BITS;
		memory.Write(frequencyHighRegister, highRegisterOther | ((frequency >> 8) & FREQUENCY_HIGH_BITS));
	}
}

#ifndef M_PI
#define M_PI  (3.14159265)
#endif

#define TABLE_SIZE   (200)

APU::APU() :
	m_channels{
		Channel(64, 4, 7,  CHANNEL1_MASTER_CONTROL_ON_OFF_BIT, CHANNEL1_CONTROL_FREQ_HIGH_REGISTER, CHANNEL1_LENGTH_DUTY_REGISTER, CHANNEL1_ENVELOPE_REGISTER),
		Channel(64, 4, 7,  CHANNEL2_MASTER_CONTROL_ON_OFF_BIT, CHANNEL2_CONTROL_FREQ_HIGH_REGISTER, CHANNEL2_LENGTH_DUTY_REGISTER, CHANNEL2_ENVELOPE_REGISTER),
		Channel(256, 2, 31, CHANNEL3_MASTER_CONTROL_ON_OFF_BIT, CHANNEL3_CONTROL_FREQ_HIGH_REGISTER, CHANNEL3_LENGTH_REGISTER, CHANNEL3_VOLUME_REGISTER),
		Channel(64, 4, 7, CHANNEL4_MASTER_CONTROL_ON_OFF_BIT, CHANNEL4_CONTROL_REGISTER, CHANNEL4_LENGTH_REGISTER, CHANNEL4_ENVELOPE_REGISTER)
}
{
	for (int i = 0; i < TABLE_SIZE; i++)
	{
		sine[i] = (float)sin(((double)i / (double)TABLE_SIZE) * M_PI * 2.);
	}

	left_phase = 0;
	right_phase = 0;

	m_externalAudioBuffer.buffer = nullptr;
	m_externalAudioBuffer.size = 0;
	m_externalAudioBuffer.sampleRate = 0;
	m_externalAudioBuffer.currentPosition = 0;
	m_externalAudioBuffer.resampleRate = 0.0f;
	m_externalAudioBuffer.samplesToGenerate = 0.0f;

	DEBUG_samplebuffer.reserve(48000);
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

	m_externalAudioBuffer.resampleRate = static_cast<double>(m_externalAudioBuffer.sampleRate) / CPU_FREQUENCY;
}

uint32_t APU::Update(Memory& memory, const uint32_t& cyclesPassed)
{
	uint32_t cyclesToStep = cyclesPassed * MCYCLES_TO_CYCLES;


	m_externalAudioBuffer.samplesToGenerate += (static_cast<double>(cyclesToStep) * m_externalAudioBuffer.resampleRate);
	uint32_t samplesgenerated = static_cast<uint32_t>(m_externalAudioBuffer.samplesToGenerate);
	/*
	while (m_externalAudioBuffer.samplesToGenerate >= 1.0f)
	{
		float* left = m_externalAudioBuffer.buffer + (*m_externalAudioBuffer.currentPosition)++;
		float* right = m_externalAudioBuffer.buffer + (*m_externalAudioBuffer.currentPosition)++;

		//*left += 0.01f;
		//*right += 0.03f;

		*left = sine[left_phase];
		*right = sine[right_phase];

		left_phase += 1;
		if (left_phase >= TABLE_SIZE) left_phase -= TABLE_SIZE;
		right_phase += 3;
		if (right_phase >= TABLE_SIZE) right_phase -= TABLE_SIZE;

		//if (*left >= 1.0f) *left -= 2.0f;
		//if (*right >= 1.0f) *right -= 2.0f;

		if (*m_externalAudioBuffer.currentPosition >= m_externalAudioBuffer.size)
		{
			*m_externalAudioBuffer.currentPosition = 0;
		}

		m_externalAudioBuffer.samplesToGenerate--;
		samplesgenerated++;
	}
*/

	if ((memory[AUDIO_MASTER_CONTROL_REGISTER] & APU_ON_OFF_BIT) == 0)
	{
		GenerateSamples(m_externalAudioBuffer, 0.0f, 0.0f);
		return samplesgenerated;
	}

	uint8_t frameSequencerStep = (memory[DIVIDER_REGISTER] / 32) % 8;
	uint32_t frameSequencerStepTmp = frameSequencerStep;
	frameSequencerStep = frameSequencerStep != m_previousFrameSequencerStep ? frameSequencerStep : FRAME_SEQUENCER_NO_TICK;
	m_previousFrameSequencerStep = frameSequencerStepTmp;



	float mixedAmplitudeLeft = 0.0f;
	float mixedAmplitudeRight = 0.0f;
	float channelsActive = 0;
	Channel* activeChannel = nullptr;
	bool isTriggered = false;

	activeChannel = m_channels + 0;
	isTriggered = CheckForTrigger(memory, *activeChannel);
	if ((*activeChannel).m_enabled)
	{	
		UpdateSweep(memory, *activeChannel, frameSequencerStep, CHANNEL1_SWEEP_REGISTER, CHANNEL1_FREQUENCY_LOW_REGISTER, CHANNEL1_CONTROL_FREQ_HIGH_REGISTER, isTriggered);
		UpdateWaveFrequency(memory, *activeChannel, CHANNEL1_FREQUENCY_LOW_REGISTER, CHANNEL1_CONTROL_FREQ_HIGH_REGISTER, cyclesToStep, isTriggered);
		UpdateLength(memory, *activeChannel, frameSequencerStep, CHANNEL1_LENGTH_DUTY_REGISTER, CHANNEL1_CONTROL_FREQ_HIGH_REGISTER, SOUND_LENGTH_TIMER_BITS, isTriggered);
		if ((*activeChannel).m_enabled)
		{
			UpdateEnvelope(memory, *activeChannel, frameSequencerStep, CHANNEL1_ENVELOPE_REGISTER, isTriggered);
			float amplitude = CalculatePulseAmplitude(memory, *activeChannel, CHANNEL1_LENGTH_DUTY_REGISTER);

			APU_Internal::Pan(memory, amplitude, 0, SOUND_PANNING_REGISTER, mixedAmplitudeLeft, mixedAmplitudeRight);
			channelsActive++;

		}
	}
	
	activeChannel = m_channels + 1;
	isTriggered = CheckForTrigger(memory, *activeChannel);
	if ((*activeChannel).m_enabled)
	{
		UpdateWaveFrequency(memory, *activeChannel, CHANNEL2_FREQUENCY_LOW_REGISTER, CHANNEL2_CONTROL_FREQ_HIGH_REGISTER, cyclesToStep, isTriggered);
		UpdateLength(memory, *activeChannel, frameSequencerStep, CHANNEL2_LENGTH_DUTY_REGISTER, CHANNEL2_CONTROL_FREQ_HIGH_REGISTER, SOUND_LENGTH_TIMER_BITS, isTriggered);
		if ((*activeChannel).m_enabled)
		{
			UpdateEnvelope(memory, *activeChannel, frameSequencerStep, CHANNEL2_ENVELOPE_REGISTER, isTriggered);
			float amplitude = CalculatePulseAmplitude(memory, *activeChannel, CHANNEL2_LENGTH_DUTY_REGISTER);
			APU_Internal::Pan(memory, amplitude, 1, SOUND_PANNING_REGISTER, mixedAmplitudeLeft, mixedAmplitudeRight);
			channelsActive++;
		}
	}
	
	activeChannel = m_channels + 2;
	isTriggered = CheckForTrigger(memory, *activeChannel);
	if ((*activeChannel).m_enabled)
	{
		UpdateWaveFrequency(memory, *activeChannel, CHANNEL3_FREQUENCY_LOW_REGISTER, CHANNEL3_CONTROL_FREQ_HIGH_REGISTER, cyclesToStep, isTriggered);
		UpdateLength(memory, *activeChannel, frameSequencerStep, CHANNEL3_LENGTH_REGISTER, CHANNEL3_CONTROL_FREQ_HIGH_REGISTER, SOUND_LENGTH_CHANNEL3_BITS, isTriggered);
		if ((*activeChannel).m_enabled)
		{
			float amplitude = CalculateAmplitude(memory, *activeChannel);
			APU_Internal::Pan(memory, amplitude, 2, SOUND_PANNING_REGISTER, mixedAmplitudeLeft, mixedAmplitudeRight);
			channelsActive++;
		}
	}
	
	mixedAmplitudeLeft /= channelsActive;
	mixedAmplitudeRight /= channelsActive;

	APU_Internal::UpdateMasterVolume(memory, mixedAmplitudeLeft, mixedAmplitudeRight);

	//TODO HPF

	GenerateSamples(m_externalAudioBuffer, mixedAmplitudeLeft, mixedAmplitudeRight);

	return samplesgenerated;
}

void APU::GenerateSamples(ExternalAudioBuffer& externalAudioBuffer, float left, float right)
{
	while (externalAudioBuffer.samplesToGenerate >= 1.0f)
	{
		APU_Internal::WriteToAudioBuffer(&externalAudioBuffer, left, right);

		if (*externalAudioBuffer.currentPosition >= externalAudioBuffer.size)
		{
			*externalAudioBuffer.currentPosition = 0;
		}

		externalAudioBuffer.samplesToGenerate--;
	}
}

bool APU::CheckForTrigger(Memory& memory, Channel& channel)
{
	bool triggered = channel.m_triggered;
	channel.m_triggered = false;
	if (triggered && channel.m_DACEnabled)
	{
		SetChannelActive(memory, channel, true);
	}
	return triggered;
}

void APU::UpdateSweep(Memory& memory, Channel& channel, uint8_t frameSequencerStep, const uint16_t& sweepRegister, const uint16_t& frequencyLow, const uint16_t& frequencyHigh, bool isTriggered)
{
	const uint8_t SWEEP_PERIOD_BITS = 0x70;
	const uint8_t SWEEP_PERIOD_OFFSET = 4;
	const uint8_t SWEEP_SHIFT_BITS = 0x07;
	const uint8_t SWEEP_DIRECTION_BIT = 0x08;
	if (isTriggered)
	{
		channel.m_shadowFrequency = APU_Internal::GetFrequency(memory, frequencyHigh, frequencyLow);
		uint8_t sweepShift = memory[sweepRegister] & SWEEP_SHIFT_BITS;
		bool periodIsZero = APU_Internal::ReloadSweepTimer(memory, channel.m_sweepTimer, sweepRegister, SWEEP_PERIOD_BITS, SWEEP_PERIOD_OFFSET);
		channel.m_sweepEnabled = !periodIsZero || sweepShift > 0;
		if (sweepShift > 0)
		{
			bool isDecreasing = memory[sweepRegister] & SWEEP_DIRECTION_BIT;
			APU_Internal::CalculateNewSweepFrequency(channel.m_enabled, channel.m_shadowFrequency, sweepShift, isDecreasing);
		}
	}
	if ((frameSequencerStep + 2) % CHANNEL1_FREQUENCY_SWEEP_TICK_RATE == 0)
	{
		if (channel.m_sweepTimer > 0)
		{
			channel.m_sweepTimer--;

			if (channel.m_sweepTimer == 0)
			{
				bool periodIsZero = APU_Internal::ReloadSweepTimer(memory, channel.m_sweepTimer, sweepRegister, SWEEP_PERIOD_BITS, SWEEP_PERIOD_OFFSET);

				if (channel.m_sweepEnabled && !periodIsZero)
				{
					uint8_t sweepShift = memory[sweepRegister] & SWEEP_SHIFT_BITS;

					bool isDecreasing = memory[sweepRegister] & SWEEP_DIRECTION_BIT;

					uint32_t newFrequency = APU_Internal::CalculateNewSweepFrequency(channel.m_enabled, channel.m_shadowFrequency, sweepShift, isDecreasing);

					if (newFrequency <= SWEEP_FREQUENCY_MAX_VALUE && sweepShift > 0)
					{
						APU_Internal::SetFrequency(memory, frequencyHigh, frequencyLow, newFrequency);
						channel.m_shadowFrequency = newFrequency;
						APU_Internal::CalculateNewSweepFrequency(channel.m_enabled, channel.m_shadowFrequency, sweepShift, isDecreasing);
					}
				}
			}
		}
	}

}

void APU::UpdateWaveFrequency(Memory& memory, Channel& channel, const uint16_t& frequencyLow, const uint16_t& frequencyHigh, const uint32_t& cyclesToStep, bool isTriggered)
{
	uint32_t frequency = APU_Internal::GetFrequency(memory, frequencyHigh, frequencyLow);

	if (isTriggered)
	{
		channel.m_frequencyTimer = (2048 - frequency) * channel.m_frequencyFactor;
		channel.m_dutyStep = IsWaveChannel(channel) ? 1 : 0;
	}	

	uint32_t remainingSteps = cyclesToStep;
	while (remainingSteps > 0)
	{
		if (remainingSteps > channel.m_frequencyTimer)
		{
			remainingSteps -= channel.m_frequencyTimer;
			channel.m_frequencyTimer = 0;
		}
		else
		{
			channel.m_frequencyTimer -= remainingSteps;
			remainingSteps = 0;
		}

		if (channel.m_frequencyTimer == 0)
		{
			channel.m_frequencyTimer = (2048 - frequency) * channel.m_frequencyFactor;
			channel.m_dutyStep++;
			channel.m_dutyStep &= channel.m_maxSampleLength;
			if (IsWaveChannel(channel))
			{
				uint16_t sampleIndex = channel.m_dutyStep / 2;
				bool sampleLowerNibble = static_cast<bool>(channel.m_dutyStep % 2);
				uint8_t doubleSample = memory[WAVE_PATTERN_RAM_BEGIN + sampleIndex];
				channel.m_sampleBuffer = sampleLowerNibble ? (doubleSample & 0x0F) : (doubleSample >> 4);
			}
		}
	}
}

void APU::UpdateEnvelope(Memory& memory, Channel& channel, uint8_t frameSequencerStep, const uint16_t& envelopeRegister, bool isTriggered)
{
	const uint8_t SWEEP_PACE_BITS = 0x07;
	const uint8_t ENVELOPE_DIRECTION_BITS = 0x8;
	const uint8_t INITIAL_VOLUME_BITS = 0xF0;
	const uint8_t INITIAL_VOLUME_OFFSET = 4;
	const uint8_t ENVELOPE_FRAME_SEQUENCER_STEP = 7;

	if (isTriggered)
	{
		uint8_t sweepPace = memory[envelopeRegister] & SWEEP_PACE_BITS;
		channel.m_periodTimer = sweepPace;
		uint8_t volume = (memory[envelopeRegister] & INITIAL_VOLUME_BITS) >> INITIAL_VOLUME_OFFSET;
		channel.m_currentVolume = volume;
		channel.DEBUG_envelopeTick = 0;
	}

	//only update envelope every 8 ticks
	if (frameSequencerStep == ENVELOPE_FRAME_SEQUENCER_STEP)
	{
		channel.DEBUG_envelopeTick++;
		uint8_t registerValue = memory[envelopeRegister];
		uint8_t period = registerValue & SWEEP_PACE_BITS;
		if (period == 0)
		{
			return;
		}
		if (channel.m_periodTimer > 0)
		{
			channel.m_periodTimer--;
		}

		if (channel.m_periodTimer == 0)
		{
			channel.m_periodTimer = period;

			bool increase = (registerValue & ENVELOPE_DIRECTION_BITS) > 0;

			if ((channel.m_currentVolume < 0x0F && increase) || (channel.m_currentVolume > 0x00 && !increase))
			{
				channel.m_currentVolume += increase ? 1 : -1;
			}
		}
	}
}

void APU::UpdateLength(Memory& memory, Channel& channel, uint8_t frameSequencerStep, const uint16_t& lengthRegister, const uint16_t& lengthEnableRegister, uint8_t lengthTimerBits, bool isTriggered)
{
	if (isTriggered)
	{
		channel.m_lengthCounter = channel.m_initialLength - (memory[lengthRegister] & lengthTimerBits);
	}

	if (frameSequencerStep % SOUND_LENGTH_TICK_RATE == 0)
	{
		bool useLength = (memory[lengthEnableRegister] & SOUND_LENGTH_ENABLE_BIT) > 0;
		if (useLength)
		{
			channel.m_lengthCounter--;
			if (channel.m_lengthCounter == 0)
			{
				SetChannelActive(memory, channel, false);
			}
		}
	}
}

float APU::CalculatePulseAmplitude(const Memory& memory,const Channel& channel, const uint16_t& dutyRegister)
{
	const uint8_t DUTY_MASK = 0xC0;
	const uint8_t DUTY_OFFSET = 0x06;
	uint8_t duty = (memory[dutyRegister] & DUTY_MASK) >> DUTY_OFFSET;
	uint8_t waveform = DUTY_WAVEFORMS[duty];
	uint8_t waveOut = (waveform >> channel.m_dutyStep) & 0x01;

	return APU_Internal::DAC(waveOut * channel.m_currentVolume);
}

float APU::CalculateAmplitude(const Memory& memory,const Channel& activeChannel)
{
	uint8_t encodedVolume = memory[CHANNEL3_VOLUME_REGISTER] >> 5;
	uint8_t volume = (encodedVolume + 3) % 4;
	if (volume == 3) volume++;
	return APU_Internal::DAC(activeChannel.m_sampleBuffer >> volume);
}

void APU::SetChannelActive(Memory& memory, Channel& channel, bool active)
{
	channel.m_enabled = active;
	uint8_t regVal = memory[AUDIO_MASTER_CONTROL_REGISTER];
	if (active)
	{
		memory.Write(AUDIO_MASTER_CONTROL_REGISTER, regVal | channel.m_masterControlOnOffBit);
	}
	else
	{
		memory.Write(AUDIO_MASTER_CONTROL_REGISTER, regVal & ~channel.m_masterControlOnOffBit);
	}
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
		Channel& channel = apu->m_channels[i];
		if (apu->m_channels[i].m_timerRegister == addr)
		{
			channel.m_lengthCounter = 64 - (newValue & SOUND_LENGTH_TIMER_BITS);
		}
	}
}

void APU::SetChannelsDACActive(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue, void* userData)
{
	const uint8_t DAC_ENABLED_BITS = 0xF8;
	APU* apu = static_cast<APU*>(userData);

	for (uint32_t i = 0; i < CHANNEL_COUNT; ++i)
	{
		Channel& channel = apu->m_channels[i];
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
	Channel& channel = apu->m_channels[2];
	channel.m_DACEnabled = newValue != 0;
	if (!channel.m_DACEnabled)
	{
		SetChannelActive(*memory, channel, false);
	}
}

bool APU::IsWaveChannel(Channel& channel)
{
	return channel.m_masterControlOnOffBit == CHANNEL3_MASTER_CONTROL_ON_OFF_BIT;
}

void APU::IsChannelTriggered(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue, void* userData)
{
	const uint8_t CONTROL_REGISTER_TRIGGER_BIT = 0x80;
	APU* apu = static_cast<APU*>(userData);

	for (uint32_t i = 0; i < CHANNEL_COUNT; ++i)
	{
		Channel& channel = apu->m_channels[i];
		if (channel.m_controlRegister != addr)
		{
			continue;
		}
		channel.m_triggered = (newValue & CONTROL_REGISTER_TRIGGER_BIT) != 0;
		break;
	}
}

