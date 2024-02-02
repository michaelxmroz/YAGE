#pragma once
#include <cstdint>
#include <vector>
#include "../Include/Emulator.h"
#include "MBC.h"
#include "Serialization.h"

#define MEMORY_SIZE 0x10000

#ifdef _DEBUG
#define TRACK_UNINITIALIZED_MEMORY_READS 1
#endif // _DEBUG

#define MAX_DELAYED_WRITES 5

class Memory;

typedef void(*MemoryWriteCallback)(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue, void* userData);

struct SpriteAttributes
{
	uint8_t m_posY;
	uint8_t m_posX;
	uint8_t m_tileIndex;
	uint8_t m_flags;
};


enum class LCDControlFlags
{
	BgEnable = 0,
	ObjEnable = 1,
	ObjSize = 2,
	BgTileMapArea = 3,
	BgTileDataArea = 4,
	WindowEnable = 5,
	WindowTileMapArea = 6,
	LCDEnable = 7
};

class Memory : ISerializable
{
public:

	enum class VRamAccess
	{
		All = 0,
		OAMBlocked = 1,
		VRamOAMBlocked = 2,
	};

	explicit Memory(Serializer* serializer);

	explicit Memory(uint8_t* rawMemory);

	~Memory();

	Memory(const Memory& other) = delete;
	Memory operator= (const Memory& other) = delete;

	uint8_t operator[](uint16_t addr) const;

	void CPUWrite(uint16_t addr, uint8_t value);
	void Write(uint16_t addr, uint8_t value, bool bypassIODelay = true);
	void WriteDirect(uint16_t addr, uint8_t value);
	void CommitDelayedWrites();
	uint8_t ReadDirect(uint16_t addr);

	const SpriteAttributes& ReadOAMEntry(uint8_t index) const;

	void ClearMemory();

	void ClearVRAM();

	void MapROM(Serializer* serializer, const char* rom, uint32_t size);
	void MapRAM(const char* ram, uint32_t size);
	void MapBootrom(const char* rom, uint32_t size);

	void RegisterCallback(uint16_t addr, MemoryWriteCallback callback, void* userData);
	void DeregisterCallback(uint16_t addr);

	void RegisterExternalRamDisableCallback(Emulator::PersistentMemoryCallback callback);

	void SetVRamAccess(VRamAccess access);
	uint8_t GetHeaderChecksum() const;

private:

	struct DelayedWrite
	{
		uint16_t m_addr;
		uint8_t m_value;
	};

	void Init();

	static void DoDMA(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue, void* userData);
	static void UnmapBootrom(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue, void* userData);

	virtual void Serialize(std::vector<Chunk>& chunks, std::vector<uint8_t>& data) override;
	virtual void Deserialize(const Chunk* chunks, const uint32_t& chunkCount, const uint8_t* data, const uint32_t& dataSize) override;

	inline void WriteInternal(uint16_t addr, uint8_t value);

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

	uint8_t* m_mappedMemory;
	uint8_t* m_romMemory;
	uint8_t* m_bootrom;
	uint8_t* m_externalRamMemory;
	MemoryWriteCallback* m_writeCallbacks;
	uint64_t* m_callbackUserData;
	MemoryBankController* m_mbc;
	Emulator::PersistentMemoryCallback m_onExternalRamDisable;
	DelayedWrite m_delayedWrites[MAX_DELAYED_WRITES];
	uint8_t m_delayedWriteCount;


	VRamAccess m_vRamAccess;
	bool m_isBootromMapped;
	bool m_externalMemory;

#ifdef TRACK_UNINITIALIZED_MEMORY_READS
	uint8_t* m_initializationTracker;
#endif
};

