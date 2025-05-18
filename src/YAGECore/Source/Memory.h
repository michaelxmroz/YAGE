#pragma once
#include "CppIncludes.h"
#include "../Include/Emulator.h"
#include "MBC.h"
#include "Serialization.h"

#define MEMORY_SIZE 0x10000
#define IOPORTS_COUNT 0x80

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

	explicit Memory(GamestateSerializer* serializer);

	explicit Memory(uint8_t* rawMemory);

	~Memory();

	Memory(const Memory& other) = delete;
	Memory operator= (const Memory& other) = delete;

	void Update();

	uint8_t operator[](uint16_t addr) const;

	void Write(uint16_t addr, uint8_t value);
	void WriteDirect(uint16_t addr, uint8_t value);
	uint8_t ReadDirect(uint16_t addr) const;

	uint8_t ReadIO(uint16_t addr) const;
	void WriteIO(uint16_t addr, uint8_t value);

	const SpriteAttributes& ReadOAMEntry(uint8_t index) const;

	void ClearMemory();
	void ClearRange(uint16_t start, uint16_t end);
	void ClearVRAM();

	void MapROM(GamestateSerializer* serializer, const char* rom, uint32_t size);
	void DeserializePersistentData(const char* ram, uint32_t size);
	void MapBootrom(const char* rom, uint32_t size);

	void RegisterCallback(uint16_t addr, MemoryWriteCallback callback, void* userData);
	void DeregisterCallback(uint16_t addr);

	void RegisterRamSaveCallback(Emulator::PersistentMemoryCallback callback);

	void SetVRamReadAccess(VRamAccess access);
	void SetVRamWriteAccess(VRamAccess access);
	uint8_t GetHeaderChecksum() const;

	void AddIOUnusedBitsOverride(uint16_t addr, uint8_t mask);
	void AddIOReadOnlyBitsOverride(uint16_t addr, uint8_t mask);
	void AddIOWriteOnlyBitsOverride(uint16_t addr, uint8_t mask);
	void AddIOReadOnlyRange(uint16_t start, uint16_t end);
	void RemoveIOReadOnlyRange(uint16_t start, uint16_t end);
	void AddIOWriteOnlyRange(uint16_t start, uint16_t end);

#if _DEBUG
	void ClearCallbacks();
	void SetMemoryCallback(uint16_t addr, Emulator::DebugCallback callback, void* userData);
	void* GetRawMemoryView();
#endif

private:

	void Init();

	static void DoDMA(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue, void* userData);
	static void UnmapBootrom(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue, void* userData);

	void Serialize(uint8_t* data) override;
	void Deserialize(const uint8_t* data) override;
	virtual uint32_t GetSerializationSize() override;

	void WriteInternal(uint16_t addr, uint8_t value);

	uint8_t CheckForIOUnusedBitOverride(uint16_t addr, uint8_t readValue) const;
	uint8_t CheckForIOWriteOnlyBitOverride(uint16_t addr, uint8_t readValue) const;
	uint8_t CheckForIOReadOnlyBitOverride(uint16_t addr, uint8_t readValue) const;

	void RegisterUnusedIORegisters();

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
	uint8_t* m_bootrom;

	MemoryWriteCallback* m_writeCallbacks;
	uint64_t* m_callbackUserData;
	MemoryBankController* m_mbc;

	VRamAccess m_vRamReadAccess;
	VRamAccess m_vRamWriteAccess;
	bool m_isBootromMapped;
	bool m_externalMemory;

	enum class DMAStatus : uint8_t
	{
		Idle,
		Initializing,
		InProgress
	};

	DMAStatus m_DMAStatus;
	uint8_t m_DMAProgress;
	bool m_DMAMemoryAccessBlocked;

	//Unused bits in IO ports return 1 when read
	uint8_t m_unusedIOBitsOverride[IOPORTS_COUNT];
	//read only bits in IO ports ignore writes
	uint8_t m_readOnlyIOBitsOverride[IOPORTS_COUNT];
	//write only bits in IO ports return 0 when read
	uint8_t m_writeOnlyIOBitsOverride[IOPORTS_COUNT];

#ifdef TRACK_UNINITIALIZED_MEMORY_READS
	uint8_t* m_initializationTracker;
#endif

#if _DEBUG
	void CheckForMemoryCallback(uint16_t addr);

	std::map<uint16_t, Emulator::DebugCallback> DEBUG_MemoryCallbackMap;
	std::map<uint16_t, void*> DEBUG_MemoryCallbackUserData;
#endif
};

