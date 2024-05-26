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

	enum class Flags : uint8_t
	{
		cy = 1 << 4,
		h = 1 << 5,
		n = 1 << 6,
		zf = 1 << 7,
		all = 0xF0
	};

	enum class State
	{
		Running = 0,
		Halt = 1,
		Stop = 2
	};

	union // Accumulator & Flags
	{
		uint16_t AF;
		struct
		{
			uint8_t FLAGS;
			uint8_t A;
		};
	};

	union // BC
	{
		uint16_t BC;
		struct
		{
			uint8_t C;
			uint8_t B;
		};
	};

	union // DE
	{
		uint16_t DE;
		struct
		{
			uint8_t E;
			uint8_t D;
		};
	};

	union // HL
	{
		uint16_t HL;
		struct
		{
			uint8_t L;
			uint8_t H;
		};
	};

	uint16_t SP; // Stack Pointer
	uint16_t PC; // Program Counter

	bool IMEF; // Interrupt Master Enable Flag

	State CpuState;

	void SetFlag(Flags flag)
	{
		FLAGS |= static_cast<uint8_t>(flag);
	}

	void SetFlag(Flags flag, uint8_t val)
	{
		FLAGS &= (~static_cast<uint8_t>(flag));
		FLAGS |= (static_cast<uint8_t>(flag) * val);
	}

	void OrFlag(Flags flag, uint8_t val)
	{
		FLAGS |= (static_cast<uint8_t>(flag) * val);
	}

	void ResetFlag(Flags flag)
	{
		FLAGS &= (~static_cast<uint8_t>(flag));
	}

	bool IsFlagSet(Flags flag) const
	{
		return (FLAGS & static_cast<uint8_t>(flag)) != 0;
	}

	uint8_t GetFlag(Flags flag) const
	{
		return static_cast<uint8_t>(IsFlagSet(flag));
	}
	
	void SetAllFlags(uint8_t val)
	{
		FLAGS &= ~static_cast<uint8_t>(Flags::all);
		FLAGS |= (val & static_cast<uint8_t>(Flags::all));
	}
	
};

