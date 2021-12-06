#pragma once
struct Registers
{
	/*
	Register F flag values
	 Bit  Name  Set Clr  Expl.
	  7    zf    Z   NZ   Zero Flag
	  6    n     -   -    Add/Sub-Flag (BCD)
	  5    h     -   -    Half Carry Flag (BCD)
	  4    cy    C   NC   Carry Flag
	  3-0  -     -   -    Not used (always zero)
	*/

	enum class Flags : unsigned char
	{
		cy = 1 << 4,
		h = 1 << 5,
		n = 1 << 6,
		zf = 1 << 7
	};

	union // Accumulator & Flags
	{
		unsigned short AF;
		unsigned char A;
		unsigned char FLAGS;
	};

	union // BC
	{
		unsigned short BC;
		unsigned char B;
		unsigned char C;
	};

	union // DE
	{
		unsigned short DE;
		unsigned char D;
		unsigned char E;
	};

	union // HL
	{
		unsigned short HL;
		unsigned char H;
		unsigned char L;
	};

	unsigned short SP; // Stack Pointer
	unsigned short PC; // Program Counter
};

