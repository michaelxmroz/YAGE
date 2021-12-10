#include "pch.h"
#include "CPU.h"

TEST(Loads8Bit, Instructions)
{
	CPU cpu;
	//LD
	{
		uint8_t command[2] = { 0x02, 0x00 };
		cpu.Reset();
		cpu.GetRegisters().A = 0x5;
		cpu.GetRegisters().BC = 0x1;
		cpu.Step(command);
		EXPECT_EQ(command[1], 0x5);
	}
	{
		uint8_t command[2] = { 0x12, 0x00 };
		cpu.Reset();
		cpu.GetRegisters().A = 0x5;
		cpu.GetRegisters().DE = 0x1;
		cpu.Step(command);
		EXPECT_EQ(command[1], 0x5);
	}
	{
		uint8_t command[2] = { 0x22, 0x00 };
		cpu.Reset();
		cpu.GetRegisters().A = 0x5;
		cpu.GetRegisters().HL = 0x1;
		cpu.Step(command);
		EXPECT_EQ(command[1], 0x5);
		EXPECT_EQ(cpu.GetRegisters().HL, 0x2);
	}
	{
		uint8_t command[2] = { 0x32, 0x00 };
		cpu.Reset();
		cpu.GetRegisters().A = 0x5;
		cpu.GetRegisters().HL = 0x1;
		cpu.Step(command);
		EXPECT_EQ(command[1], 0x5);
		EXPECT_EQ(cpu.GetRegisters().HL, 0x0);
	}
	{
		uint8_t command[2] = { 0x06, 0x05 };
		cpu.Reset();
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().B, 0x5);
	}
	{
		uint8_t command[2] = { 0x16, 0x05 };
		cpu.Reset();
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().D, 0x5);
	}
	{
		uint8_t command[2] = { 0x26, 0x05 };
		cpu.Reset();
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().H, 0x5);
	}
	{
		uint8_t command[3] = { 0x36, 0x05, 0x0 };
		cpu.Reset();
		cpu.GetRegisters().HL = 0x2;
		cpu.Step(command);
		EXPECT_EQ(command[2], 0x05);
	}
}

TEST(Loads16Bit, Instructions) 
{
	CPU cpu;
	//LD
	{
		uint8_t command[3] = { 0x01, 0x05 , 0x01 };
		cpu.Reset();
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().BC, 0x0105);
	}
	{
		uint8_t command[3] = { 0x11, 0x05 , 0x01 };
		cpu.Reset();
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().DE, 0x0105);
	}
	{
		uint8_t command[3] = { 0x21, 0x05 , 0x01 };
		cpu.Reset();
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().HL, 0x0105);
	}
	{
		uint8_t command[3] = { 0x31, 0x05 , 0x01 };
		cpu.Reset();
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().SP, 0x0105);
	}
}

TEST(Arithmatic16Bit, Instructions)
{
	CPU cpu;
	//INC
	{
		uint8_t command[1] = { 0x03 };
		cpu.Reset();
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().BC, 0x1);
	}
	{
		uint8_t command[1] = { 0x13 };
		cpu.Reset();
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().DE, 0x1);
	}
	{
		uint8_t command[1] = { 0x23 };
		cpu.Reset();
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().HL, 0x1);
	}
	{
		uint8_t command[1] = { 0x33 };
		cpu.Reset();
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().SP, 0xFFFF);
	}
}

TEST(Arithmatic8Bit, Instructions)
{
	CPU cpu;
	//INC
	{
		uint8_t command[1] = { 0x04 };
		cpu.Reset();
		cpu.GetRegisters().B = 0xFF;
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().B, 0x0);
		EXPECT_TRUE(cpu.GetRegisters().IsFlagSet(Registers::Flags::zf));
		EXPECT_TRUE(cpu.GetRegisters().IsFlagSet(Registers::Flags::h));
	}
	{
		uint8_t command[1] = { 0x14 };
		cpu.Reset();
		cpu.GetRegisters().D = 0xFF;
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().D, 0x0);
		EXPECT_TRUE(cpu.GetRegisters().IsFlagSet(Registers::Flags::zf));
	}
	{
		uint8_t command[1] = { 0x24 };
		cpu.Reset();
		cpu.GetRegisters().H = 0xFF;
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().H, 0x0);
		EXPECT_TRUE(cpu.GetRegisters().IsFlagSet(Registers::Flags::zf));
	}
	{
		uint8_t command[2] = { 0x34, 0xFF };
		cpu.Reset();
		cpu.GetRegisters().HL = 0x1;
		cpu.Step(command);
		EXPECT_EQ(command[1], 0x0);
		EXPECT_TRUE(cpu.GetRegisters().IsFlagSet(Registers::Flags::zf));
	}
	//DEC
	{
		uint8_t command[1] = { 0x05 };
		cpu.Reset();
		cpu.GetRegisters().B = 0x1;
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().B, 0x0);
		EXPECT_TRUE(cpu.GetRegisters().IsFlagSet(Registers::Flags::zf));
		EXPECT_FALSE(cpu.GetRegisters().IsFlagSet(Registers::Flags::h));
	}
	{
		uint8_t command[1] = { 0x15 };
		cpu.Reset();
		cpu.GetRegisters().D = 0x0;
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().D, 0xFF);
		EXPECT_FALSE(cpu.GetRegisters().IsFlagSet(Registers::Flags::zf));
		EXPECT_TRUE(cpu.GetRegisters().IsFlagSet(Registers::Flags::h));
	}
	{
		uint8_t command[1] = { 0x25 };
		cpu.Reset();
		cpu.GetRegisters().H = 0x1;
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().H, 0x0);
		EXPECT_TRUE(cpu.GetRegisters().IsFlagSet(Registers::Flags::zf));
	}
	{
		uint8_t command[2] = { 0x35, 0x1 };
		cpu.Reset();
		cpu.GetRegisters().HL = 0x1;
		cpu.Step(command);
		EXPECT_EQ(command[1], 0x0);
		EXPECT_TRUE(cpu.GetRegisters().IsFlagSet(Registers::Flags::zf));
	}
}

TEST(Shift8Bit, Instructions)
{
	CPU cpu;
	{
		uint8_t command[1] = { 0x07 };
		cpu.Reset();
		cpu.GetRegisters().A = 0x81;
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().A, 0x2);
		EXPECT_TRUE(cpu.GetRegisters().IsFlagSet(Registers::Flags::cy));
	}
	{
		uint8_t command[1] = { 0x17 };
		cpu.Reset();
		cpu.GetRegisters().A = 0x81;
		cpu.GetRegisters().SetFlag(Registers::Flags::cy);
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().A, 0x3);
		EXPECT_TRUE(cpu.GetRegisters().IsFlagSet(Registers::Flags::cy));
	}
}