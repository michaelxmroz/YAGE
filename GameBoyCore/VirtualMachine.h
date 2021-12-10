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
	: m_memory()
	{
		m_memory = new uint8_t[MEMORY_SIZE];
	}

	~VirtualMachine()
	{
		delete[] m_memory;
	}

	VirtualMachine(const VirtualMachine& other) = delete;
	VirtualMachine operator= (const VirtualMachine& other) = delete;

	bool Load(std::shared_ptr<std::vector<char>> romBlob)
	{
		m_romBlob = romBlob;

		return true;
	}

	bool Start()
	{
		// Setup memory
		ClearMemory();
		MapROM();
		m_cpu.Reset();
		m_cpu.SetProgramCounter(0);

		// Run
		while (true)
		{
			m_cpu.Step(m_memory);

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
		memset(m_memory, 0, MEMORY_SIZE);
	}

	void MapROM()
	{
		memcpy(m_memory, &((*m_romBlob)[0]), m_romBlob->size());
	}

	std::shared_ptr<std::vector<char>> m_romBlob;

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

	uint8_t* m_memory;

	CPU m_cpu;

};

