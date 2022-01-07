#pragma once
#include <cstdint>
#include <vector>

#define MEMORY_SIZE 0x10000

class Memory
{
public:
	Memory();

	explicit Memory(uint8_t* rawMemory);

	~Memory();

	Memory(const Memory& other) = delete;
	Memory operator= (const Memory& other) = delete;

	const uint8_t& operator[](uint16_t index) const
	{
		return m_memory[index];
	}

	uint8_t& Write(uint16_t index);

	void WriteDirect(uint16_t index, uint8_t value);

	void ClearMemory();

	void MapROM(std::vector<char>* m_romBlob);

private:

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

	bool m_externalMemory;
};

