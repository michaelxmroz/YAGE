#include "pch.h"
#include "Memory.h"
#include "CPU.h"

TEST(DMATests, DMA1)
{
	CPU cpu;
	{
		uint8_t* command = new uint8_t[0x10000];
		memset(command, 0, 0x10000);
		Memory mem(command);
		cpu.Reset();

		command[0] = 0x36;
		command[1] = 0x0;
		command[2] = 0xCD;
		cpu.GetRegisters().HL = 0xFF46;

		cpu.Step(mem);

		EXPECT_EQ(command[0xFE00], 0x36);
		EXPECT_EQ(command[0xFE02], 0xCD);

		delete[] command;
	}
}

TEST(DMATests, DMA2)
{
	CPU cpu;
	{
		uint8_t* command = new uint8_t[0x10000];
		memset(command, 0, 0x10000);
		Memory mem(command);
		cpu.Reset();

		command[0] = 0x36;
		command[1] = 0x2;
		command[0x200] = 0xCD;
		cpu.GetRegisters().HL = 0xFF46;

		cpu.Step(mem);

		EXPECT_EQ(command[0xFE00], 0xCD);
		EXPECT_EQ(command[0xFE02], 0x00);

		delete[] command;
	}
}