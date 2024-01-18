#include "Helpers.h"
#include "APU.h"

#define CHANNEL1_SWEEP_REGISTER 0xFF10
#define CHANNEL1_LENGTH_DUTY_REGISTER 0xFF11
#define CHANNEL1_ENVELOPE_REGISTER 0xFF12
#define CHANNEL1_FREQUENCY_LOW_REGISTER 0xFF13
#define CHANNEL1_FREQUENCY_HIGH_REGISTER 0xFF14

#define CHANNEL2_LENGTH_DUTY_REGISTER 0xFF16
#define CHANNEL2_ENVELOPE_REGISTER 0xFF17
#define CHANNEL2_FREQUENCY_LOW_REGISTER 0xFF18
#define CHANNEL2_FREQUENCY_HIGH_REGISTER 0xFF19

#define CHANNEL3_ON_OFF_REGISTER 0xFF1A
#define CHANNEL3_LENGTH_REGISTER 0xFF1B
#define CHANNEL3_OUTPUT_LEVEL_REGISTER 0xFF1C
#define CHANNEL3_FREQUENCY_LOW_REGISTER 0xFF1D
#define CHANNEL3_FREQUENCY_HIGH_REGISTER 0xFF1E

#define CHANNEL4_LENGTH_REGISTER 0xFF20
#define CHANNEL4_ENVELOPE_REGISTER 0xFF21
#define CHANNEL4_POLY_COUNTER_REGISTER 0xFF22
#define CHANNEL4_CONSEC_COUNTER_REGISTER 0xFF23

#define CHANNEL_CONTROL_REGISTER 0xFF24
#define SOUND_OUTPUT_TERMINAL_REGISTER 0xFF25
#define SOUND_ON_OFF_REGISTER 0xFF26

#define WAVE_PATTERN_RAM_BEGIN 0xFF30
#define WAVE_PATTERN_RAM_END 0xFF3F

#define DIVIDER_REGISTER 0xFF04

#define MCYCLES_TO_CYCLES 4

const uint8_t DUTY_WAVEFORMS[4]
{
	0b00000001,
	0b10000001,
	0b10000111,
	0b01111110,
};

namespace APU_Internal
{
	void CheckForReset(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue)
	{
		if ((prevValue & 0x80) != 0 && (prevValue & 0x80) == 0)
		{

			//TODO reset all other registers & counters
		}
	}

	float DAC(uint8_t amplitude)
	{
		return static_cast<float>(amplitude) / 7.5f - 1.0f;
	}
}

APU::APU()
{
	m_externalAudioBuffer.buffer = nullptr;
	m_externalAudioBuffer.size = 0;
	m_externalAudioBuffer.sampleRate = 0;
	m_externalAudioBuffer.currentPosition = 0;
}

void APU::Init(Memory& memory)
{
	memory.RegisterCallback(SOUND_ON_OFF_REGISTER, APU_Internal::CheckForReset);
}

void APU::SetExternalAudioBuffer(float* buffer, uint32_t size, uint32_t sampleRate, uint32_t* startOffset)
{
	m_externalAudioBuffer.buffer = buffer;
	m_externalAudioBuffer.size = size;
	m_externalAudioBuffer.sampleRate = sampleRate;
	m_externalAudioBuffer.currentPosition = startOffset;

	m_externalAudioBuffer.resampleRate = static_cast<float>(m_externalAudioBuffer.sampleRate) / CPU_FREQUENCY;
	
}

void APU::Update(Memory& memory, const uint32_t& cyclesPassed)
{
	if ((memory[SOUND_ON_OFF_REGISTER] & 0x80) == 0)
	{
		return;
	}

	uint8_t frameSequencerStep = (memory[DIVIDER_REGISTER] / 32) % 8;

	uint32_t cyclesToStep = cyclesPassed * 4;

	float mixedAmplitudeLeft = 0.0f;
	float mixedAmplitudeRight = 0.0f;

	bool isTriggered = CheckForTrigger(memory, m_channels[1], CHANNEL2_FREQUENCY_HIGH_REGISTER);
	if(m_channels[1].m_enabled)
	{
		UpdateTimer(memory, m_channels[1], CHANNEL2_FREQUENCY_LOW_REGISTER, CHANNEL2_FREQUENCY_HIGH_REGISTER, cyclesToStep, isTriggered);
		UpdateEnvelope(memory, m_channels[1], frameSequencerStep, CHANNEL2_ENVELOPE_REGISTER, isTriggered);
		UpdateLength(memory, m_channels[1], frameSequencerStep, CHANNEL2_LENGTH_DUTY_REGISTER, CHANNEL2_FREQUENCY_HIGH_REGISTER, isTriggered, false);
		float amplitude = CalculateWaveAmplitude(memory, m_channels[1], CHANNEL2_LENGTH_DUTY_REGISTER);
		Pan(memory, m_channels[1], amplitude, 1, SOUND_OUTPUT_TERMINAL_REGISTER, mixedAmplitudeLeft, mixedAmplitudeRight);
	}

	uint32_t samplesToGenerate = cyclesPassed * MCYCLES_TO_CYCLES * static_cast<uint32_t>(ceil(m_externalAudioBuffer.resampleRate));
	while (samplesToGenerate > 0)
	{
		float* left = m_externalAudioBuffer.buffer + (*m_externalAudioBuffer.currentPosition)++;
		float* right = m_externalAudioBuffer.buffer + (*m_externalAudioBuffer.currentPosition)++;
		
		*left += 0.01f;
		*right += 0.03f;

		if (*left >= 1.0f) *left -= 2.0f;
		if (*right >= 1.0f) *right -= 2.0f;

		if (*m_externalAudioBuffer.currentPosition >= m_externalAudioBuffer.size)
		{
			*m_externalAudioBuffer.currentPosition = 0;
		}

		samplesToGenerate--;
	}
	
	//TODO channels 1,3,4

}

bool APU::CheckForTrigger(Memory& memory, Channel& channel, const uint16_t& triggerRegister)
{
	uint8_t regVal = memory[triggerRegister];
	bool trigger = (regVal & 0x80) > 0;

	if (trigger)
	{
		channel.m_enabled = true;
		memory.Write(triggerRegister, regVal & 0x7F);
	}

	//TODO is this right?
	return trigger;
}

void APU::UpdateTimer(Memory& memory, Channel& channel, const uint16_t& frequencyLow, const uint16_t& frequencyHigh, const uint32_t& cyclesToStep, bool isTriggered)
{
	if (isTriggered)
	{
		uint32_t frequency = ((memory[frequencyHigh] & 0x07) << 8) | memory[frequencyLow];
		channel.m_frequencyTimer = (2048 - frequency) * 4;
		channel.m_wavePosition = 0;
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
			uint32_t frequency = ((memory[frequencyHigh] & 0x07) << 8) | memory[frequencyLow];
			channel.m_frequencyTimer = (2048 - frequency) * 4;
			channel.m_wavePosition++;
			channel.m_wavePosition &= 0x7;
		}
	}
}

void APU::UpdateEnvelope(Memory& memory, Channel& channel, uint8_t frameSequencerStep, const uint16_t& envelopeRegister, bool isTriggered)
{
	if (isTriggered)
	{
		uint8_t period = memory[envelopeRegister] & 0x07;
		channel.m_periodTimer = period;
		uint8_t volume = (memory[envelopeRegister] & 0xF0) >> 4;
		channel.m_currentVolume = volume;
	}

	if (frameSequencerStep == 7)
	{
		uint8_t registerValue = memory[envelopeRegister];
		uint8_t period = registerValue & 0x07;
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

			bool increase = (registerValue & 0x8) > 0;

			if ((channel.m_currentVolume < 0xF && increase) || (channel.m_currentVolume > 0x0 && !increase))
			{
				channel.m_currentVolume += increase ? 1 : -1;
			}
		}
	}
}

void APU::UpdateLength(Memory& memory, Channel& channel, uint8_t frameSequencerStep, const uint16_t& lengthRegister, const uint16_t& lengthEnableRegister, bool isTriggered, bool isWave)
{
	if (isTriggered)
	{
		if (!isWave)
		{
			channel.m_lengthCounter = 64 - (memory[lengthRegister] & 0x3F);
		}
	}

	if (frameSequencerStep % 2 == 0)
	{
		bool useLength = (memory[lengthEnableRegister] & 0x40) > 0;
		if (useLength)
		{
			channel.m_lengthCounter--;
			if (channel.m_lengthCounter == 0)
			{
				channel.m_enabled = false;
			}
		}
	}
}

float APU::CalculateWaveAmplitude(Memory& memory, Channel& channel, const uint16_t& dutyRegister)
{
	uint8_t duty = (memory[dutyRegister] & 0xC0) >> 6;
	uint8_t waveform = DUTY_WAVEFORMS[duty];
	uint8_t waveOut = (waveform >> channel.m_wavePosition) & 0x01;

	return APU_Internal::DAC(waveOut * channel.m_currentVolume);
}

void APU::Pan(Memory& memory, Channel& channel, float amplitude, uint8_t panRegisterOffset, const uint16_t& panRegister, float& mixedAmplitudeLeft, float& mixedAmplitudeRight)
{
	bool hasLeft = ((memory[panRegister] >> panRegisterOffset) & 0x01) > 0;
	bool hasRight = ((memory[panRegister] >> (panRegisterOffset + 4)) & 0x01) > 0;

	mixedAmplitudeLeft += hasLeft ? amplitude : 0.0f;
	mixedAmplitudeRight += hasRight ? amplitude : 0.0f;
}
