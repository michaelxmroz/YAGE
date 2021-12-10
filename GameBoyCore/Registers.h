#pragma once
#include "Helpers.h"

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
		struct
		{
			unsigned char A;
			unsigned char FLAGS;
		};
	};

	union // BC
	{
		unsigned short BC;
		struct
		{
			unsigned char B;
			unsigned char C;
		};
	};

	union // DE
	{
		unsigned short DE;
		struct
		{
			unsigned char D;
			unsigned char E;
		};
	};

	union // HL
	{
		unsigned short HL;
		struct
		{
			unsigned char H;
			unsigned char L;
		};
	};

	unsigned short SP; // Stack Pointer
	unsigned short PC; // Program Counter

	FORCE_INLINE void SetFlag(Flags flag)
	{
		FLAGS |= static_cast<uint8_t>(flag);
	}

	FORCE_INLINE void SetFlag(Flags flag, uint8_t val)
	{
		FLAGS &= (~static_cast<uint8_t>(flag));
		FLAGS |= (static_cast<uint8_t>(flag) * val);
	}

	FORCE_INLINE void ResetFlag(Flags flag)
	{
		FLAGS &= (~static_cast<uint8_t>(flag));
	}

	FORCE_INLINE bool IsFlagSet(Flags flag) const
	{
		return (FLAGS & static_cast<uint8_t>(flag)) != 0;
	}

	FORCE_INLINE uint8_t GetFlag(Flags flag) const
	{
		return static_cast<uint8_t>(IsFlagSet(flag));
	}
};

