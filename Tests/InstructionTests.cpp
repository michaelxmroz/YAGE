#include "pch.h"
#include "CPU.h"

TEST(Loads8Bit, Instructions)
{
	CPU cpu;

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
}

TEST(Loads16Bit, Instructions) 
{
	CPU cpu;

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