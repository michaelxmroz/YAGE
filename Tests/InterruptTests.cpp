#include "pch.h"
#include "CPU.h"
#include "Interrupts.h"

TEST(Interrupts, Vblank)
{
	CPU cpu(true);
	{
		uint8_t* command = new uint8_t[0x10000];
		memset(command, 0, 0x10000);

		cpu.Reset();
		cpu.GetRegisters().IMEF = true;
		Memory mem(command);
		Interrupts::RequestInterrupt(Interrupts::Types::VBlank, mem);
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().PC, 0x0001);
		EXPECT_EQ(cpu.GetRegisters().SP, 0xFFFE);
		EXPECT_TRUE(cpu.GetRegisters().IMEF);
		delete[] command;
	}
	{
		uint8_t* command = new uint8_t[0x10000];
		memset(command, 0, 0x10000);

		cpu.Reset();
		cpu.GetRegisters().IMEF = true;
		Memory mem(command);
		Interrupts::EnableInterrupt(Interrupts::Types::VBlank, mem);
		Interrupts::RequestInterrupt(Interrupts::Types::VBlank, mem);
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().PC, 0x0040);
		EXPECT_EQ(cpu.GetRegisters().SP, 0xFFFC);
		EXPECT_FALSE(cpu.GetRegisters().IMEF);
		delete[] command;
	}
}

TEST(Interrupts, LCD_STAT)
{
	CPU cpu(true);
	{
		uint8_t* command = new uint8_t[0x10000];
		memset(command, 0, 0x10000);

		cpu.Reset();
		cpu.GetRegisters().IMEF = true;
		Memory mem(command);
		Interrupts::RequestInterrupt(Interrupts::Types::LCD_STAT, mem);
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().PC, 0x0001);
		EXPECT_EQ(cpu.GetRegisters().SP, 0xFFFE);
		EXPECT_TRUE(cpu.GetRegisters().IMEF);
		delete[] command;
	}
	{
		uint8_t* command = new uint8_t[0x10000];
		memset(command, 0, 0x10000);

		cpu.Reset();
		cpu.GetRegisters().IMEF = true;
		Memory mem(command);
		Interrupts::EnableInterrupt(Interrupts::Types::LCD_STAT, mem);
		Interrupts::RequestInterrupt(Interrupts::Types::LCD_STAT, mem);
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().PC, 0x0048);
		EXPECT_EQ(cpu.GetRegisters().SP, 0xFFFC);
		EXPECT_FALSE(cpu.GetRegisters().IMEF);
		delete[] command;
	}
}

TEST(Interrupts, Timer)
{
	CPU cpu(true);
	{
		uint8_t* command = new uint8_t[0x10000];
		memset(command, 0, 0x10000);

		cpu.Reset();
		cpu.GetRegisters().IMEF = true;
		Memory mem(command);
		Interrupts::RequestInterrupt(Interrupts::Types::Timer, mem);
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().PC, 0x0001);
		EXPECT_EQ(cpu.GetRegisters().SP, 0xFFFE);
		EXPECT_TRUE(cpu.GetRegisters().IMEF);
		delete[] command;
	}
	{
		uint8_t* command = new uint8_t[0x10000];
		memset(command, 0, 0x10000);

		cpu.Reset();
		cpu.GetRegisters().IMEF = true;
		Memory mem(command);
		Interrupts::EnableInterrupt(Interrupts::Types::Timer, mem);
		Interrupts::RequestInterrupt(Interrupts::Types::Timer, mem);
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().PC, 0x0050);
		EXPECT_EQ(cpu.GetRegisters().SP, 0xFFFC);
		EXPECT_FALSE(cpu.GetRegisters().IMEF);
		delete[] command;
	}
}

TEST(Interrupts, Serial)
{
	CPU cpu(true);
	{
		uint8_t* command = new uint8_t[0x10000];
		memset(command, 0, 0x10000);

		cpu.Reset();
		cpu.GetRegisters().IMEF = true;
		Memory mem(command);
		Interrupts::RequestInterrupt(Interrupts::Types::Serial, mem);
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().PC, 0x0001);
		EXPECT_EQ(cpu.GetRegisters().SP, 0xFFFE);
		EXPECT_TRUE(cpu.GetRegisters().IMEF);
		delete[] command;
	}
	{
		uint8_t* command = new uint8_t[0x10000];
		memset(command, 0, 0x10000);

		cpu.Reset();
		cpu.GetRegisters().IMEF = true;
		Memory mem(command);
		Interrupts::EnableInterrupt(Interrupts::Types::Serial, mem);
		Interrupts::RequestInterrupt(Interrupts::Types::Serial, mem);
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().PC, 0x0058);
		EXPECT_EQ(cpu.GetRegisters().SP, 0xFFFC);
		EXPECT_FALSE(cpu.GetRegisters().IMEF);
		delete[] command;
	}
}

TEST(Interrupts, Joypad)
{
	CPU cpu(true);
	{
		uint8_t* command = new uint8_t[0x10000];
		memset(command, 0, 0x10000);

		cpu.Reset();
		cpu.GetRegisters().IMEF = true;
		Memory mem(command);
		Interrupts::RequestInterrupt(Interrupts::Types::Joypad, mem);
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().PC, 0x0001);
		EXPECT_EQ(cpu.GetRegisters().SP, 0xFFFE);
		EXPECT_TRUE(cpu.GetRegisters().IMEF);
		delete[] command;
	}
	{
		uint8_t* command = new uint8_t[0x10000];
		memset(command, 0, 0x10000);

		cpu.Reset();
		cpu.GetRegisters().IMEF = true;
		Memory mem(command);
		Interrupts::EnableInterrupt(Interrupts::Types::Joypad, mem);
		Interrupts::RequestInterrupt(Interrupts::Types::Joypad, mem);
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().PC, 0x0060);
		EXPECT_EQ(cpu.GetRegisters().SP, 0xFFFC);
		EXPECT_FALSE(cpu.GetRegisters().IMEF);
		delete[] command;
	}
}

TEST(Interrupts, Priority)
{
	CPU cpu(true);
	{
		uint8_t* command = new uint8_t[0x10000];
		memset(command, 0, 0x10000);

		cpu.Reset();
		cpu.GetRegisters().IMEF = true;
		Memory mem(command);
		Interrupts::EnableInterrupt(Interrupts::Types::VBlank, mem);
		Interrupts::RequestInterrupt(Interrupts::Types::VBlank, mem);

		Interrupts::EnableInterrupt(Interrupts::Types::LCD_STAT, mem);
		Interrupts::RequestInterrupt(Interrupts::Types::LCD_STAT, mem);
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().PC, 0x0040);
		EXPECT_EQ(cpu.GetRegisters().SP, 0xFFFC);
		EXPECT_FALSE(cpu.GetRegisters().IMEF);
		delete[] command;
	}
}

TEST(Interrupts, EI_delay)
{
	CPU cpu(true);
	{
		uint8_t* command = new uint8_t[0x10000];
		memset(command, 0, 0x10000);
		cpu.Reset();

		cpu.GetRegisters().IMEF = false;
		command[0] = 0xFB;
		Memory mem(command);
		Interrupts::RequestInterrupt(Interrupts::Types::Joypad, mem);
		Interrupts::EnableInterrupt(Interrupts::Types::Joypad, mem);
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().PC, 0x0001);
		EXPECT_EQ(cpu.GetRegisters().SP, 0xFFFE);
		EXPECT_TRUE(cpu.GetRegisters().IMEF);
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().PC, 0x0060);
		EXPECT_EQ(cpu.GetRegisters().SP, 0xFFFC);
		EXPECT_FALSE(cpu.GetRegisters().IMEF);
		delete[] command;
	}
}

TEST(Interrupts, HALT_Continue)
{
	CPU cpu(true);
	{
		uint8_t* command = new uint8_t[0x10000];
		memset(command, 0, 0x10000);
		cpu.Reset();

		cpu.GetRegisters().IMEF = true;
		command[0] = 0x76;
		cpu.Step(command);
		cpu.Step(command);
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().PC, 0x0001);
		EXPECT_EQ(cpu.GetRegisters().SP, 0xFFFE);
		EXPECT_TRUE(cpu.GetRegisters().IMEF);
		Memory mem(command);
		EXPECT_EQ(cpu.GetRegisters().CpuState, Registers::State::Halt);

		Interrupts::RequestInterrupt(Interrupts::Types::VBlank, mem);
		Interrupts::EnableInterrupt(Interrupts::Types::VBlank, mem);
		cpu.Step(command);

		EXPECT_EQ(cpu.GetRegisters().PC, 0x0040);
		EXPECT_EQ(cpu.GetRegisters().SP, 0xFFFC);
		EXPECT_FALSE(cpu.GetRegisters().IMEF);
		EXPECT_EQ(cpu.GetRegisters().CpuState, Registers::State::Running);
		delete[] command;
	}
}

TEST(Interrupts, HALT_Bug)
{
	CPU cpu(true);
	{
		uint8_t* command = new uint8_t[0x10000];
		memset(command, 0, 0x10000);
		cpu.Reset();

		cpu.GetRegisters().IMEF = false;
		command[0] = 0x76;
		cpu.Step(command);
		cpu.Step(command);
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().PC, 0x0001);
		EXPECT_EQ(cpu.GetRegisters().SP, 0xFFFE);
		EXPECT_FALSE(cpu.GetRegisters().IMEF);
		EXPECT_EQ(cpu.GetRegisters().CpuState, Registers::State::Halt);

		Memory mem(command);
		Interrupts::RequestInterrupt(Interrupts::Types::VBlank, mem);
		Interrupts::EnableInterrupt(Interrupts::Types::VBlank, mem);
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().CpuState, Registers::State::Running);

		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().PC, 0x0001);
		EXPECT_EQ(cpu.GetRegisters().SP, 0xFFFE);
		EXPECT_FALSE(cpu.GetRegisters().IMEF);
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().PC, 0x0002);
		delete[] command;
	}
}
