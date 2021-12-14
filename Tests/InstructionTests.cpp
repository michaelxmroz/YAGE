#include "pch.h"
#include "CPU.h"

uint16_t u16(uint8_t lsb, uint8_t msb)
{
	return static_cast<uint16_t>(msb) << 8 | static_cast<uint16_t>(lsb);
}

TEST(Loads8Bit, Instructions)
{
	CPU cpu;
	//LD A to memory
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
	//LD immediate to register
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
	{
		uint8_t command[2] = { 0x0E, 0x05 };
		cpu.Reset();
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().C, 0x5);
	}
	{
		uint8_t command[2] = { 0x1E, 0x05 };
		cpu.Reset();
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().E, 0x5);
	}
	{
		uint8_t command[2] = { 0x2E, 0x05 };
		cpu.Reset();
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().L, 0x5);
	}
	{
		uint8_t command[2] = { 0x3E, 0x05 };
		cpu.Reset();
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().A, 0x5);
	}
	//LD memory to A
	{
		uint8_t command[2] = { 0x0A, 0x17 };
		cpu.Reset();
		cpu.GetRegisters().BC = 0x1;
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().A, 0x17);
	}
	{
		uint8_t command[2] = { 0x1A, 0x17 };
		cpu.Reset();
		cpu.GetRegisters().DE = 0x1;
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().A, 0x17);
	}
	{
		uint8_t command[2] = { 0x2A, 0x17 };
		cpu.Reset();
		cpu.GetRegisters().HL = 0x1;
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().A, 0x17);
		EXPECT_EQ(cpu.GetRegisters().HL, 0x2);
	}
	{
		uint8_t command[2] = { 0x3A, 0x17 };
		cpu.Reset();
		cpu.GetRegisters().HL = 0x1;
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().A, 0x17);
		EXPECT_EQ(cpu.GetRegisters().HL, 0x0);
	}
	//LD reg to reg, reg to memory, memory to reg
	{
		uint8_t command[8] = { 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47 };
		cpu.Reset();
		cpu.GetRegisters().C = 0x1;
		cpu.GetRegisters().D = 0x2;
		cpu.GetRegisters().E = 0x3;
		cpu.GetRegisters().H = 0x4;
		cpu.GetRegisters().L = 0x5;
		cpu.GetRegisters().A = 0x6;

		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().B, 0x0);
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().B, 0x1);
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().B, 0x2);
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().B, 0x3);
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().B, 0x4);
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().B, 0x5);

		cpu.GetRegisters().HL = 0x1;
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().B, 0x41);
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().B, 0x6);
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
	{
		uint8_t command[5] = { 0x08, 0x03, 0x00, 0x00, 0x00 };
		cpu.Reset();
		cpu.Step(command);
		EXPECT_EQ(u16(command[3], command[4]), cpu.GetRegisters().SP);
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
	//ADD
	{
		uint8_t command[1] = { 0x09 };
		cpu.Reset();
		cpu.GetRegisters().HL = 0x988;
		cpu.GetRegisters().BC = 0x980;
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().HL, 0x1308);
		EXPECT_TRUE(cpu.GetRegisters().IsFlagSet(Registers::Flags::h));
	}
	{
		uint8_t command[1] = { 0x19 };
		cpu.Reset();
		cpu.GetRegisters().HL = 0x988;
		cpu.GetRegisters().DE = 0x980;
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().HL, 0x1308);
		EXPECT_TRUE(cpu.GetRegisters().IsFlagSet(Registers::Flags::h));
	}
	{
		uint8_t command[1] = { 0x29 };
		cpu.Reset();
		cpu.GetRegisters().HL = 0x988;
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().HL, 0x1310);
		EXPECT_TRUE(cpu.GetRegisters().IsFlagSet(Registers::Flags::h));
	}
	{
		uint8_t command[1] = { 0x39 };
		cpu.Reset();
		cpu.GetRegisters().HL = 0x988;
		cpu.GetRegisters().SP = 0x980;
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().HL, 0x1308);
		EXPECT_TRUE(cpu.GetRegisters().IsFlagSet(Registers::Flags::h));
	}
	//DEC
	{
		uint8_t command[1] = { 0x0B };
		cpu.Reset();
		cpu.GetRegisters().BC = 0x02;
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().BC, 0x1);
	}
	{
		uint8_t command[1] = { 0x1B };
		cpu.Reset();
		cpu.GetRegisters().DE = 0x02;
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().DE, 0x1);
	}
	{
		uint8_t command[1] = { 0x2B };
		cpu.Reset();
		cpu.GetRegisters().HL = 0x02;
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().HL, 0x1);
	}
	{
		uint8_t command[1] = { 0x3B };
		cpu.Reset();
		cpu.GetRegisters().SP = 0x02;
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().SP, 0x1);
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
	{
		uint8_t command[1] = { 0x0C };
		cpu.Reset();
		cpu.GetRegisters().C = 0xFF;
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().C, 0x0);
		EXPECT_TRUE(cpu.GetRegisters().IsFlagSet(Registers::Flags::zf));
		EXPECT_TRUE(cpu.GetRegisters().IsFlagSet(Registers::Flags::h));
	}
	{
		uint8_t command[1] = { 0x1C };
		cpu.Reset();
		cpu.GetRegisters().E = 0xFF;
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().E, 0x0);
		EXPECT_TRUE(cpu.GetRegisters().IsFlagSet(Registers::Flags::zf));
	}
	{
		uint8_t command[1] = { 0x2C };
		cpu.Reset();
		cpu.GetRegisters().L = 0xFF;
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().L, 0x0);
		EXPECT_TRUE(cpu.GetRegisters().IsFlagSet(Registers::Flags::zf));
	}
	{
		uint8_t command[1] = { 0x3C };
		cpu.Reset();
		cpu.GetRegisters().A = 0x1;
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().A, 0x02);
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
	{
		uint8_t command[1] = { 0x0D };
		cpu.Reset();
		cpu.GetRegisters().C = 0x1;
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().C, 0x0);
		EXPECT_TRUE(cpu.GetRegisters().IsFlagSet(Registers::Flags::zf));
		EXPECT_FALSE(cpu.GetRegisters().IsFlagSet(Registers::Flags::h));
	}
	{
		uint8_t command[1] = { 0x1D };
		cpu.Reset();
		cpu.GetRegisters().E = 0x0;
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().E, 0xFF);
		EXPECT_FALSE(cpu.GetRegisters().IsFlagSet(Registers::Flags::zf));
		EXPECT_TRUE(cpu.GetRegisters().IsFlagSet(Registers::Flags::h));
	}
	{
		uint8_t command[1] = { 0x2D };
		cpu.Reset();
		cpu.GetRegisters().L = 0x1;
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().L, 0x0);
		EXPECT_TRUE(cpu.GetRegisters().IsFlagSet(Registers::Flags::zf));
	}
	{
		uint8_t command[1] = { 0x3D };
		cpu.Reset();
		cpu.GetRegisters().A = 0x1;
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().A, 0x0);
		EXPECT_TRUE(cpu.GetRegisters().IsFlagSet(Registers::Flags::zf));
	}
	//CPL
	{
		uint8_t command[1] = { 0x2F };
		cpu.Reset();
		cpu.GetRegisters().A = 0x3;
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().A, 0xFC);
		EXPECT_TRUE(cpu.GetRegisters().IsFlagSet(Registers::Flags::n));
		EXPECT_TRUE(cpu.GetRegisters().IsFlagSet(Registers::Flags::h));
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
	{
		uint8_t command[1] = { 0x0F };
		cpu.Reset();
		cpu.GetRegisters().A = 0x81;
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().A, 0x40);
		EXPECT_TRUE(cpu.GetRegisters().IsFlagSet(Registers::Flags::cy));
	}
	{
		uint8_t command[1] = { 0x1F };
		cpu.Reset();
		cpu.GetRegisters().A = 0x81;
		cpu.GetRegisters().SetFlag(Registers::Flags::cy);
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().A, 0xC0);
		EXPECT_TRUE(cpu.GetRegisters().IsFlagSet(Registers::Flags::cy));
	}
}

TEST(Control, Instructions)
{
	CPU cpu;
	// Carry set/toggle
	{
		uint8_t command[1] = { 0x37 };
		cpu.Reset();
		cpu.Step(command);
		EXPECT_TRUE(cpu.GetRegisters().IsFlagSet(Registers::Flags::cy));
		EXPECT_FALSE(cpu.GetRegisters().IsFlagSet(Registers::Flags::n));
		EXPECT_FALSE(cpu.GetRegisters().IsFlagSet(Registers::Flags::h));
	}
	{
		uint8_t command[1] = { 0x3F };
		cpu.Reset();
		cpu.GetRegisters().SetFlag(Registers::Flags::cy);
		cpu.Step(command);
		EXPECT_FALSE(cpu.GetRegisters().IsFlagSet(Registers::Flags::cy));
		EXPECT_FALSE(cpu.GetRegisters().IsFlagSet(Registers::Flags::n));
		EXPECT_FALSE(cpu.GetRegisters().IsFlagSet(Registers::Flags::h));
	}
	//Jumps
	{
		uint8_t command[2] = { 0x20, 0x01 };
		cpu.Reset();
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().PC, 0x03);

		cpu.Reset();
		cpu.GetRegisters().SetFlag(Registers::Flags::zf);
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().PC, 0x02);
	}
	{
		uint8_t command[2] = { 0x30, 0x01 };
		cpu.Reset();
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().PC, 0x03);

		cpu.Reset();
		cpu.GetRegisters().SetFlag(Registers::Flags::cy);
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().PC, 0x02);
	}
	{
		uint8_t command[2] = { 0x18, 0x05 };
		cpu.Reset();
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().PC, 0x07);
	}
	{
		uint8_t command[2] = { 0x28, 0x01 };
		cpu.Reset();
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().PC, 0x02);

		cpu.Reset();
		cpu.GetRegisters().SetFlag(Registers::Flags::zf);
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().PC, 0x03);
	}
	{
		uint8_t command[2] = { 0x38, 0x01 };
		cpu.Reset();
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().PC, 0x02);

		cpu.Reset();
		cpu.GetRegisters().SetFlag(Registers::Flags::cy);
		cpu.Step(command);
		EXPECT_EQ(cpu.GetRegisters().PC, 0x03);
	}
}