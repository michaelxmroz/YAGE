#include "AudioChannel.h"

void AudioProcessors::Sweep::UpdateSweep(Memory& memory, ChannelData& channel, uint8_t frameSequencerStep, bool isTriggered)
{
	const uint8_t SWEEP_PERIOD_BITS = 0x70;
	const uint8_t SWEEP_PERIOD_OFFSET = 4;
	const uint8_t SWEEP_SHIFT_BITS = 0x07;
	const uint8_t SWEEP_DIRECTION_BIT = 0x08;
	const uint32_t FREQUENCY_SWEEP_TICK_RATE = 4;

	if (isTriggered)
	{
		channel.m_shadowFrequency = AudioChannel_Internal::GetFrequency(memory, channel.m_controlRegister, channel.m_frequencyRegister);
		uint8_t sweepShift = memory[channel.m_sweepRegister] & SWEEP_SHIFT_BITS;
		bool periodIsZero = ReloadSweepTimer(memory, channel.m_sweepTimer, channel.m_sweepRegister, SWEEP_PERIOD_BITS, SWEEP_PERIOD_OFFSET);
		channel.m_sweepEnabled = !periodIsZero || sweepShift > 0;
		if (sweepShift > 0)
		{
			bool isDecreasing = memory[channel.m_sweepRegister] & SWEEP_DIRECTION_BIT;
			CalculateNewSweepFrequency(channel.m_enabled, channel.m_shadowFrequency, sweepShift, isDecreasing);
		}
	}
	if ((frameSequencerStep + 2) % FREQUENCY_SWEEP_TICK_RATE == 0)
	{
		if (channel.m_sweepTimer > 0)
		{
			channel.m_sweepTimer--;

			if (channel.m_sweepTimer == 0)
			{
				bool periodIsZero = ReloadSweepTimer(memory, channel.m_sweepTimer, channel.m_sweepRegister, SWEEP_PERIOD_BITS, SWEEP_PERIOD_OFFSET);

				if (channel.m_sweepEnabled && !periodIsZero)
				{
					uint8_t sweepShift = memory[channel.m_sweepRegister] & SWEEP_SHIFT_BITS;

					bool isDecreasing = memory[channel.m_sweepRegister] & SWEEP_DIRECTION_BIT;

					uint32_t newFrequency = CalculateNewSweepFrequency(channel.m_enabled, channel.m_shadowFrequency, sweepShift, isDecreasing);

					if (newFrequency <= Sweep::SWEEP_FREQUENCY_MAX_VALUE && sweepShift > 0)
					{
						AudioChannel_Internal::SetFrequency(memory, channel.m_controlRegister, channel.m_frequencyRegister, newFrequency);
						channel.m_shadowFrequency = newFrequency;
						CalculateNewSweepFrequency(channel.m_enabled, channel.m_shadowFrequency, sweepShift, isDecreasing);
					}
				}
			}
		}
	}

}

bool AudioProcessors::Sweep::ReloadSweepTimer(const Memory& memory, uint32_t& sweepTimer, uint16_t sweepRegister, uint8_t sweepPeriodBits, uint8_t sweepPeriodOffset)
{
	const uint32_t SWEEP_PERIOD_DEFAULT_VALUE = 8;
	sweepTimer = (memory[sweepRegister] & sweepPeriodBits) >> sweepPeriodOffset;
	if (sweepTimer == 0)
	{
		sweepTimer = SWEEP_PERIOD_DEFAULT_VALUE;
		return true;
	}
	return false;
}

uint32_t AudioProcessors::Sweep::CalculateNewSweepFrequency(bool& channelActive, uint32_t shadowFrequency, uint8_t sweepShift, bool isDecreasing)
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

	// overflow check
	if (newFrequency > SWEEP_FREQUENCY_MAX_VALUE)
	{
		channelActive = false;
	}

	return newFrequency;
}

void AudioProcessors::PulseFrequency::UpdateFrequency(Memory& memory, ChannelData& channel, uint32_t cyclesToStep, bool isTriggered)
{
	UpdateFrequencyInternal(memory, channel, cyclesToStep, isTriggered, 0);
}

bool AudioProcessors::PulseFrequency::UpdateFrequencyInternal(Memory& memory, ChannelData& channel, uint32_t cyclesToStep, bool isTriggered, uint32_t initialDutyStep)
{
	uint32_t frequency = AudioChannel_Internal::GetFrequency(memory, channel.m_controlRegister, channel.m_frequencyRegister);

	if (isTriggered)
	{
		channel.m_frequencyTimer = (2048 - frequency) * channel.m_frequencyFactor;
		channel.m_dutyStep = initialDutyStep;
	}

	uint32_t remainingSteps = cyclesToStep;
	bool timerTriggered = false;
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
			timerTriggered = true;
		}
	}
	return timerTriggered;
}

void AudioProcessors::WaveFrequency::UpdateFrequency(Memory& memory, ChannelData& channel, const uint32_t& cyclesToStep, bool isTriggered)
{
	const uint16_t WAVE_PATTERN_RAM_BEGIN = 0xFF30;
	if (PulseFrequency::UpdateFrequencyInternal(memory, channel, cyclesToStep, isTriggered, 1))
	{
		uint16_t sampleIndex = channel.m_dutyStep / 2;
		bool sampleLowerNibble = static_cast<bool>(channel.m_dutyStep % 2);
		uint8_t doubleSample = memory[WAVE_PATTERN_RAM_BEGIN + sampleIndex];
		channel.m_sampleBuffer = sampleLowerNibble ? (doubleSample & 0x0F) : (doubleSample >> 4);
	}
}

void AudioProcessors::NoiseFrequency::UpdateFrequency(Memory& memory, ChannelData& channel, const uint32_t& cyclesToStep, bool isTriggered)
{
	const uint8_t LFSR_WIDTH_BITS = 0x08;
	const uint8_t LFSR_WIDTH_OFFSET = 3;

	if (isTriggered)
	{
		channel.m_lfsr = 0;
		channel.m_frequencyTimer = GetNoiseFrequencyTimer(memory[channel.m_frequencyRegister]);
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
			channel.m_frequencyTimer = GetNoiseFrequencyTimer(memory[channel.m_frequencyRegister]);

			uint16_t shiftRegister = channel.m_lfsr;
			uint16_t bit1 = shiftRegister & 0x1;
			uint16_t bit2 = (shiftRegister >> 1) & 0x1;
			uint16_t nXorResult = ~(bit1 ^ bit2);

			bool shortWidth = (memory[channel.m_frequencyRegister] & LFSR_WIDTH_BITS) >> LFSR_WIDTH_OFFSET;

			if (shortWidth)
			{
				uint16_t shiftMask = shiftRegister & 0x7F7F;
				shiftRegister = nXorResult << 15 | shiftMask;
				shiftRegister = nXorResult << 7 | shiftRegister;
			}
			else
			{
				uint16_t shiftMask = shiftRegister & 0x7FFF;
				shiftRegister = nXorResult << 15 | shiftMask;
			}

			shiftRegister >>= 1;

			channel.m_lfsr = shiftRegister;
		}
	}
}

uint32_t AudioProcessors::NoiseFrequency::GetNoiseFrequencyTimer(uint8_t frequencyRegister)
{
	const uint8_t CLOCK_SHIFT_BITS = 0xF0;
	const uint8_t CLOCK_SHIFT_OFFSET = 4;
	const uint8_t CLOCK_DIVIDER_BITS = 0x07;

	uint32_t clockDivider = frequencyRegister & CLOCK_DIVIDER_BITS;
	uint32_t clockShift = (frequencyRegister & CLOCK_SHIFT_BITS) >> CLOCK_SHIFT_OFFSET;
	return (clockDivider > 0 ? (clockDivider << 4) : 8) << clockShift;
}

void AudioProcessors::Length::UpdateLength(Memory& memory, ChannelData& channel, uint8_t frameSequencerStep, bool isTriggered)
{
	const uint8_t SOUND_LENGTH_TICK_RATE = 2;
	const uint8_t SOUND_LENGTH_ENABLE_BIT = 0x40;

	if (isTriggered)
	{
		channel.m_lengthCounter = channel.m_initialLength - (memory[channel.m_timerRegister] & channel.m_lengthTimerBits);
	}

	if (frameSequencerStep % SOUND_LENGTH_TICK_RATE == 0)
	{
		bool useLength = (memory[channel.m_controlRegister] & SOUND_LENGTH_ENABLE_BIT) > 0;
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

void AudioProcessors::Envelope::UpdateVolume(Memory& memory, ChannelData& channel, uint8_t frameSequencerStep, bool isTriggered)
{
	const uint8_t SWEEP_PACE_BITS = 0x07;
	const uint8_t ENVELOPE_DIRECTION_BITS = 0x8;
	const uint8_t INITIAL_VOLUME_BITS = 0xF0;
	const uint8_t INITIAL_VOLUME_OFFSET = 4;
	const uint8_t ENVELOPE_FRAME_SEQUENCER_STEP = 7;

	if (isTriggered)
	{
		uint8_t sweepPace = memory[channel.m_volumeEnvelopeRegister] & SWEEP_PACE_BITS;
		channel.m_periodTimer = sweepPace;
		uint8_t volume = (memory[channel.m_volumeEnvelopeRegister] & INITIAL_VOLUME_BITS) >> INITIAL_VOLUME_OFFSET;
		channel.m_currentVolume = volume;
	}

	//only update envelope every 8 ticks
	if (frameSequencerStep == ENVELOPE_FRAME_SEQUENCER_STEP)
	{
		uint8_t registerValue = memory[channel.m_volumeEnvelopeRegister];
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

float AudioProcessors::PulseAmplitude::UpdateAmplitude(const Memory& memory, const ChannelData& channel)
{
	const uint8_t DUTY_WAVEFORMS[4]
	{
		0b00000001,
		0b10000001,
		0b10000111,
		0b01111110,
	};
	const uint8_t DUTY_MASK = 0xC0;
	const uint8_t DUTY_OFFSET = 0x06;
	uint8_t duty = (memory[channel.m_timerRegister] & DUTY_MASK) >> DUTY_OFFSET;
	uint8_t waveform = DUTY_WAVEFORMS[duty];
	uint8_t waveOut = (waveform >> channel.m_dutyStep) & 0x01;

	return AudioChannel_Internal::DAC(waveOut * channel.m_currentVolume);
}

float AudioProcessors::WaveAmplitude::UpdateAmplitude(const Memory& memory, const ChannelData& channel)
{
	uint8_t encodedVolume = memory[channel.m_volumeEnvelopeRegister] >> 5;
	uint8_t volume = (encodedVolume + 3) % 4;
	if (volume == 3) volume++;
	return AudioChannel_Internal::DAC(channel.m_sampleBuffer >> volume);
}

float AudioProcessors::NoiseAmplitude::UpdateAmplitude(const Memory& memory, const ChannelData& channel)
{
	uint32_t isActive = channel.m_lfsr & 0x1;
	return AudioChannel_Internal::DAC(channel.m_currentVolume * isActive);
}

float AudioProcessors::AudioChannel_Internal::DAC(uint8_t amplitude)
{
	//return -(static_cast<float>(amplitude) / 7.5f - 1.0f);
	return static_cast<float>(amplitude) / 15.0f;
}

void AudioProcessors::AudioChannel_Internal::Pan(Memory& memory, float amplitude, uint8_t panRegisterOffset, float& mixedAmplitudeLeft, float& mixedAmplitudeRight)
{
	const uint16_t PAN_REGISTER = 0xFF25;
	bool hasRight = ((memory[PAN_REGISTER] >> panRegisterOffset) & 0x01) > 0;
	bool hasLeft = ((memory[PAN_REGISTER] >> (panRegisterOffset + 4)) & 0x01) > 0;

	mixedAmplitudeLeft += hasLeft ? amplitude : 0.0f;
	mixedAmplitudeRight += hasRight ? amplitude : 0.0f;
}

uint32_t AudioProcessors::AudioChannel_Internal::GetFrequency(const Memory& memory, const uint16_t& frequencyHighRegister, const uint16_t& frequencyLowRegister)
{
	const uint8_t FREQUENCY_HIGH_BITS = 0x07;
	return ((static_cast<uint32_t>(memory[frequencyHighRegister]) & FREQUENCY_HIGH_BITS) << 8) | memory[frequencyLowRegister];
}

void AudioProcessors::AudioChannel_Internal::SetFrequency(Memory& memory, const uint16_t& frequencyHighRegister, const uint16_t& frequencyLowRegister, uint32_t frequency)
{
	const uint8_t FREQUENCY_HIGH_BITS = 0x07;
	const uint8_t FREQUENCY_LOW_BITS = 0x0F;
	memory.Write(frequencyLowRegister, frequency & FREQUENCY_LOW_BITS);
	uint8_t highRegisterOther = memory[frequencyHighRegister] & ~FREQUENCY_HIGH_BITS;
	memory.Write(frequencyHighRegister, highRegisterOther | ((frequency >> 8) & FREQUENCY_HIGH_BITS));
}

bool AudioProcessors::AudioChannel_Internal::CheckForTrigger(Memory& memory, ChannelData& channel)
{
	bool triggered = channel.m_triggered;
	channel.m_triggered = false;
	if (triggered && channel.m_DACEnabled)
	{
		SetChannelActive(memory, channel, true);
	}
	return triggered;
}

void AudioProcessors::SetChannelActive(Memory& memory, ChannelData& channel, bool active)
{
	const uint16_t AUDIO_MASTER_CONTROL_REGISTER = 0xFF26;
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
