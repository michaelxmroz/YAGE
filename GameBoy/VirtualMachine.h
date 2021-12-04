#pragma once
#include <vector>
#include <memory>
#include "Registers.h"
#include "CPU.h"

#define MEMORY_SIZE 0xFFFF
#define ROM_ENTRY_POINT 0x0100

class VirtualMachine
{
public:
	VirtualMachine()
	{
	}

	bool Load(std::shared_ptr<std::vector<char>> romBlob)
	{
		_romBlob = romBlob;

		return true;
	}

	bool Start()
	{
		// Setup memory
		ClearMemory();
		MapROM();
		_cpu.Reset();
		_cpu.SetProgramCounter(ROM_ENTRY_POINT);

		// Run
		while (true)
		{
			_cpu.Step(_memory);

			//TODO process I/O
			//TODO process PPU
			//TODO process APU
			//TODO render
			//TODO audio
		}
		
		return true;
	}

private:

	void ClearMemory()
	{
		memset(_memory, 0, MEMORY_SIZE);
	}

	void MapROM()
	{
		memcpy(_memory, &((*_romBlob)[0]), _romBlob->size());
	}

	std::shared_ptr<std::vector<char>> _romBlob;

	/*
	  Memory Map

	  0000-3FFF   16KB ROM Bank 00     (in cartridge, fixed at bank 00)
	  4000-7FFF   16KB ROM Bank 01..NN (in cartridge, switchable bank number)
	  8000-9FFF   8KB Video RAM (VRAM) (switchable bank 0-1 in CGB Mode)
	  A000-BFFF   8KB External RAM     (in cartridge, switchable bank, if any)
	  C000-CFFF   4KB Work RAM Bank 0 (WRAM)
	  D000-DFFF   4KB Work RAM Bank 1 (WRAM)  (switchable bank 1-7 in CGB Mode)
	  E000-FDFF   Same as C000-DDFF (ECHO)    (typically not used)
	  FE00-FE9F   Sprite Attribute Table (OAM)
	  FEA0-FEFF   Not Usable
	  FF00-FF7F   I/O Ports
	  FF80-FFFE   High RAM (HRAM)
	  FFFF        Interrupt Enable Register
	*/

	uint8_t _memory[MEMORY_SIZE];

	CPU _cpu;

};

