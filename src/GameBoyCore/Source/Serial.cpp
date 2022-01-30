#include "Serial.h"
#include "Helpers.h"

#define SB_REGISTER 0xFF01
#define SC_REGISTER 0xFF02

Serial::Serial()
{
}

void Serial::Init(Memory& memory)
{
	memory.Write(SB_REGISTER, 0x00);
	memory.Write(SB_REGISTER, 0x7E);
}
