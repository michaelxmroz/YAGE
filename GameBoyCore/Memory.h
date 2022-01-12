#pragma once
#include <cstdint>
#include <vector>

#define MEMORY_SIZE 0x10000

#ifdef _DEBUG
#define TRACK_UNINITIALIZED_MEMORY_READS 1
#endif // _DEBUG


class Memory;

typedef void(*MemoryWriteCallback)(Memory* memory);

struct SpriteAttributes
{
	uint8_t m_posY;
	uint8_t m_posX;
	uint8_t m_tileIndex;
	uint8_t m_flags;
};

class Memory
{
public:

	enum class VRamAccess
	{
		All = 0,
		OAMBlocked = 1,
		VRamOAMBlocked = 2,
	};

	Memory();

	explicit Memory(uint8_t* rawMemory);

	~Memory();

	Memory(const Memory& other) = delete;
	Memory operator= (const Memory& other) = delete;

	uint8_t operator[](uint16_t addr) const;

	void Write(uint16_t addr, uint8_t value);
	void WriteDirect(uint16_t addr, uint8_t value);
	uint8_t ReadDirect(uint16_t addr);

	const SpriteAttributes& ReadOAMEntry(uint8_t index) const;

	void ClearMemory();

	void MapROM(std::vector<char>* m_romBlob);

	void RegisterCallback(uint16_t addr, MemoryWriteCallback callback);
	void DeregisterCallback(uint16_t addr);

	void SetVRamAccess(VRamAccess access);

private:

	void Init();

	static void DoDMA(Memory* memory);

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
	MemoryWriteCallback* m_writeCallbacks;

	VRamAccess m_vRamAccess;
	bool m_externalMemory;

#ifdef TRACK_UNINITIALIZED_MEMORY_READS
	uint8_t* m_initializationTracker;
#endif
};

