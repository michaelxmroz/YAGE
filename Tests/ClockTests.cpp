#include "pch.h"
#include "Memory.h"
#include "CPU.h"
#include "Clock.h"
#include "Interrupts.h"

TEST(ClockTest, Divider)
{
	CPU cpu(true);
	{
		Clock clock;
		uint8_t* command = new uint8_t[0x10000];
		memset(command, 0, 0x10000);
		Memory mem(command);
		cpu.Reset();
		
		EXPECT_EQ(command[0xFF04], 0x0);
		
		for (uint32_t i = 0; i < 65; ++i)
		{
			uint32_t mCycles = cpu.Step(mem);
			clock.Increment(mCycles, mem);
		}
		
		EXPECT_EQ(command[0xFF04], 0x1);

		delete[] command;
	}
}

TEST(ClockTest, Timer)
{
	CPU cpu(true);
	{
		Clock clock;
		uint8_t* command = new uint8_t[0x10000];
		memset(command, 0, 0x10000);
		Memory mem(command);

		cpu.Reset();

		EXPECT_EQ(command[0xFF05], 0x0);

		for (uint32_t i = 0; i < 65; ++i)
		{
			uint32_t mCycles = cpu.Step(mem);
			clock.Increment(mCycles, mem);
		}

		EXPECT_EQ(command[0xFF05], 0x0);

		command[0xFF07] = 0x6;
		clock.Reset();
		for (uint32_t i = 0; i < 65; ++i)
		{
			uint32_t mCycles = cpu.Step(mem);
			clock.Increment(mCycles, mem);
		}

		EXPECT_EQ(command[0xFF05], 0x4);

		Interrupts::EnableInterrupt(Interrupts::Types::Timer, mem);

		command[0xFF07] = 0x5;
		command[0xFF05] = 0xFF;
		clock.Reset();
		for (uint32_t i = 0; i < 4; ++i)
		{
			uint32_t mCycles = cpu.Step(mem);
			clock.Increment(mCycles, mem);
		}
		EXPECT_EQ(command[0xFF05], 0x0);
		EXPECT_TRUE(Interrupts::ShouldHandleInterrupt(Interrupts::Types::Timer, mem));
		Interrupts::ClearInterruptRequest(Interrupts::Types::Timer, mem);
		command[0xFF05] = 0xFF;
		command[0xFF06] = 0xCC;
		clock.Reset();
		for (uint32_t i = 0; i < 4; ++i)
		{
			uint32_t mCycles = cpu.Step(mem);
			clock.Increment(mCycles, mem);
		}
		EXPECT_EQ(command[0xFF05], 0xCC);
		EXPECT_TRUE(Interrupts::ShouldHandleInterrupt(Interrupts::Types::Timer, mem));

		delete[] command;
	}
}