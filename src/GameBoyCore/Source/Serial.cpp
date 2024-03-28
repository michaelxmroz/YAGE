#include "Serial.h"
#include "Helpers.h"
#include "Interrupts.h"
#include "Logging.h"

#define SB_REGISTER 0xFF01
#define SC_REGISTER 0xFF02

#define SC_TRANSFER_ENABLE_MASK 0x80

#define TRANSFER_CLOCK_MCYCLES 32

Serial::Serial()
{
	m_accumulatedCycles = 0;
	m_bitsTransferred = 0;
	m_mode = SerialMode::None;
}

void Serial::Init(Memory& memory)
{
	memory.Write(SB_REGISTER, 0x00);
	memory.Write(SC_REGISTER, 0x7E);

	memory.RegisterCallback(SC_REGISTER, ResetClock, this);
}

void Serial::Update(Memory& memory, uint32_t mCycles)
{
	if(m_mode == SerialMode::None)
	{
		return;
	}

	if ((memory[SC_REGISTER] & SC_TRANSFER_ENABLE_MASK) > 0)
	{
		m_accumulatedCycles += mCycles;

		if (m_accumulatedCycles >= TRANSFER_CLOCK_MCYCLES)
		{
			m_accumulatedCycles -= TRANSFER_CLOCK_MCYCLES;
			TransferNextBit(memory);
		}
	}
	
}

void Serial::ResetClock(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue, void* userData)
{
	Serial* serial = static_cast<Serial*>(userData);
	if ((newValue & SC_TRANSFER_ENABLE_MASK) > 0 && (prevValue & SC_TRANSFER_ENABLE_MASK) == 0)
	{
		serial->m_accumulatedCycles = 0;
		serial->m_bitsTransferred = 0;
	}
}

void Serial::TransferNextBit(Memory& memory)
{
	//Loopback transfer
	uint8_t sb = memory[SB_REGISTER];

	const uint8_t lastBit = (sb & 0x80) >> 7;
	sb <<= 1;
	sb |= lastBit;

	memory.Write(SB_REGISTER, sb);

	m_bitsTransferred++;

	if (m_bitsTransferred == 8)
	{
		uint8_t sc = memory[SC_REGISTER];
		sc &= ~SC_TRANSFER_ENABLE_MASK;
		memory.Write(SC_REGISTER, sc);
		Interrupts::RequestInterrupt(Interrupts::Types::Serial, memory);

		LOG_INFO(string_format("Serial transfer complete. Transferred: 0x%02X", sb).c_str());
	}
}
