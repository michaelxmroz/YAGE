#include "Memory.h"
#include "Clock.h"
#include "Logging.h"

#define ECHO_RAM_BEGIN 0xE000
#define ECHO_RAM_END 0xE000
#define UNUSABLE_BEGIN 0xFEA0
#define UNUSABLE_END 0xFEFF
#define DMA_REGISTER 0xFF46
#define OAM_START 0xFE00
#define OAM_END 0xFE9F
#define OAM_SIZE 0xA0
#define VRAM_START 0x8000
#define VRAM_END 0x9FFF 

#define BOOTROM_BANK 0xFF50
#define BOOTROM_SIZE 0x100

#define HEADER_CHECKSUM 0x014D
#define HEADER_CARTRIDGE_TYPE 0x0147
#define HEADER_ROM_SIZE 0x0148
#define HEADER_RAM_SIZE 0x0149

Memory::Memory(Serializer* serializer) : ISerializable(serializer)
{
	m_mappedMemory = new uint8_t[MEMORY_SIZE];
	m_externalMemory = false;
	Init();
}

Memory::Memory(uint8_t* rawMemory) : ISerializable(nullptr)
{
	m_mappedMemory = rawMemory;
	m_externalMemory = true;
	Init();
}

Memory::~Memory()
{
	if (!m_externalMemory)
	{
		delete[] m_mappedMemory;
		delete m_mbc;
	}
	if (m_bootrom != nullptr)
	{
		delete[] m_bootrom;
	}
	if (m_romMemory != nullptr)
	{
		delete[] m_romMemory;
	}

	if (m_externalRamMemory != nullptr)
	{
		delete[] m_externalRamMemory;
	}

	delete[] m_writeCallbacks;
}

void Memory::Write(uint16_t addr, uint8_t value)
{
	if (!m_externalMemory)
	{
		if (m_vRamAccess != VRamAccess::All)
		{
			if (addr >= VRAM_START && addr <= VRAM_END && m_vRamAccess == VRamAccess::VRamOAMBlocked)
			{
				return;
			}
			if (addr >= OAM_START && addr <= OAM_END)
			{
				return;
			}
		}

		if (addr <= ROM_END)
		{
			if (m_mbc->WriteRegister(addr, value))
			{
				if (m_onExternalRamDisable != nullptr)
				{
					m_onExternalRamDisable(m_externalRamMemory, m_mbc->GetRAMSize());
				}
			}
			return;
		}
		else if (addr >= EXTERNAL_RAM_BEGIN && addr < EXTERNAL_RAM_BEGIN + RAM_BANK_SIZE)
		{
			if(m_externalRamMemory != nullptr)
			{
				m_externalRamMemory[m_mbc->GetRAMAddr(addr)] = value;
			}
			else
			{
				LOG_WARNING(string_format("Trying to write to non-existent RAM addr %x", addr).c_str());
			}
		}
		else if (addr >= ECHO_RAM_BEGIN && addr <= ECHO_RAM_END)
		{
			LOG_INFO(string_format("Trying to write echo RAM addr %x", addr).c_str());
		}
		else if (addr >= UNUSABLE_BEGIN && addr <= UNUSABLE_END)
		{
			LOG_WARNING(string_format("Trying to write unusable addr %x", addr).c_str());
			return;
		}
	}

	uint8_t prevValue = m_mappedMemory[addr];
	m_mappedMemory[addr] = value;

	if (m_writeCallbacks[addr] != nullptr)
	{
		m_writeCallbacks[addr](this, addr, prevValue, value, reinterpret_cast<void*>(m_callbackUserData[addr]));
	}

#ifdef TRACK_UNINITIALIZED_MEMORY_READS
	m_initializationTracker[addr] = 1;
#endif
}

void Memory::WriteDirect(uint16_t addr, uint8_t value)
{
	m_mappedMemory[addr] = value;

#ifdef TRACK_UNINITIALIZED_MEMORY_READS
	m_initializationTracker[addr] = 1;
#endif
}

uint8_t Memory::ReadDirect(uint16_t addr)
{
	return m_mappedMemory[addr];
}

const SpriteAttributes& Memory::ReadOAMEntry(uint8_t index) const
{
	return reinterpret_cast<SpriteAttributes*>(m_mappedMemory + OAM_START)[index];
}

void Memory::ClearMemory()
{
	memset(m_mappedMemory, 0, MEMORY_SIZE);
#ifdef TRACK_UNINITIALIZED_MEMORY_READS
	memset(m_initializationTracker, 0, MEMORY_SIZE);
	//skip initialization checks for APU wave ram
	memset(m_initializationTracker + 0xFF30, 1, 0xFF3F - 0xFF30 + 1);
#endif

	WriteDirect(DMA_REGISTER, 0xFF);
}

void Memory::ClearVRAM()
{
	memset(m_mappedMemory + VRAM_START, 0, VRAM_END - VRAM_START);
#ifdef TRACK_UNINITIALIZED_MEMORY_READS
	memset(m_initializationTracker + VRAM_START, 1, VRAM_END - VRAM_START);
#endif
}

void Memory::MapROM(Serializer* serializer, const char* rom, uint32_t size)
{
	m_romMemory = new uint8_t[size];
	memcpy(m_romMemory, rom, size);

	m_mbc = new MemoryBankController(serializer, m_romMemory[HEADER_CARTRIDGE_TYPE], m_romMemory[HEADER_ROM_SIZE], m_romMemory[HEADER_RAM_SIZE]);

	uint16_t ramSize = m_mbc->GetRAMSize();
	if (ramSize > 0)
	{
		m_externalRamMemory = new uint8_t[ramSize];
		memset(m_externalRamMemory, 0, ramSize);
	}

#ifdef TRACK_UNINITIALIZED_MEMORY_READS
	memset(m_initializationTracker, 1, ROM_END + 1);
	memset(m_initializationTracker + EXTERNAL_RAM_BEGIN, 1, RAM_BANK_SIZE);
#endif
}

void Memory::MapRAM(const char* ram, uint32_t size)
{
	uint32_t expectedRAMSize = static_cast<uint32_t>(m_mbc->GetRAMSize());
	if (size != expectedRAMSize)
	{
		LOG_ERROR("Mismatching external RAM sizes");
	}

	memcpy(m_externalRamMemory, ram, size);
}

void Memory::MapBootrom(const char* rom, uint32_t size)
{
	m_bootrom = new uint8_t[BOOTROM_SIZE];
	memcpy(m_bootrom, rom, size);
	m_isBootromMapped = true;
}

void Memory::RegisterCallback(uint16_t addr, MemoryWriteCallback callback, void* userData)
{
	m_writeCallbacks[addr] = callback;
	m_callbackUserData[addr] = reinterpret_cast<uint64_t>(userData);
}

void Memory::DeregisterCallback(uint16_t addr)
{
	m_writeCallbacks[addr] = nullptr;
	m_callbackUserData[addr] = 0;
}

void Memory::RegisterExternalRamDisableCallback(Emulator::PersistentMemoryCallback callback)
{
	m_onExternalRamDisable = callback;
}

void Memory::SetVRamAccess(VRamAccess access)
{
	m_vRamAccess = access;
}

uint8_t Memory::GetHeaderChecksum() const
{
	return m_romMemory[HEADER_CHECKSUM];
}

void Memory::Init()
{
	m_romMemory = nullptr;
	m_externalRamMemory = nullptr;
	m_isBootromMapped = false;
	m_bootrom = nullptr;
	m_onExternalRamDisable = nullptr;
	m_mbc = nullptr;

	m_writeCallbacks = new MemoryWriteCallback[MEMORY_SIZE];
	memset(m_writeCallbacks, 0, sizeof(MemoryWriteCallback) * MEMORY_SIZE);

	m_callbackUserData = new uint64_t[MEMORY_SIZE];
	memset(m_callbackUserData, 0, sizeof(uint64_t) * MEMORY_SIZE);

	RegisterCallback(DMA_REGISTER, DoDMA, nullptr);
	RegisterCallback(BOOTROM_BANK, UnmapBootrom, nullptr);

	m_vRamAccess = VRamAccess::All;

#ifdef TRACK_UNINITIALIZED_MEMORY_READS
	m_initializationTracker = new uint8_t[MEMORY_SIZE];
	memset(m_initializationTracker, 0, MEMORY_SIZE);
	//skip initialization checks for APU wave ram
	memset(m_initializationTracker + 0xFF30, 1, 0xFF3F - 0xFF30 + 1);
#endif
}

void Memory::DoDMA(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue, void* userData)
{
	uint16_t source = static_cast<uint16_t>((*memory)[DMA_REGISTER]) << 8;
	//TODO DMA from external ram?
	if (source < ROM_END && !memory->m_externalMemory)
	{
		memcpy(memory->m_mappedMemory + OAM_START, memory->m_romMemory + source, OAM_SIZE);
	}
	else
	{
		memcpy(memory->m_mappedMemory + OAM_START, memory->m_mappedMemory + source, OAM_SIZE);
	}

#ifdef TRACK_UNINITIALIZED_MEMORY_READS
	memset(memory->m_initializationTracker + OAM_START, 1, OAM_SIZE);
#endif
}

void Memory::UnmapBootrom(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue, void* userData)
{
	memory->m_isBootromMapped = false;
}

uint8_t Memory::operator[](uint16_t addr) const
{
	if (m_vRamAccess != VRamAccess::All)
	{
		if (addr >= VRAM_START && addr <= VRAM_END && m_vRamAccess == VRamAccess::VRamOAMBlocked)
		{
			return 0xFF;
		}
		if (addr >= OAM_START && addr <= OAM_END)
		{
			return 0xFF;
		}
	}

#ifdef TRACK_UNINITIALIZED_MEMORY_READS
	if (m_initializationTracker[addr] == 0 && !m_externalMemory)
	{
		LOG_ERROR(string_format("Uninitialized memory read at location %x", addr).c_str());
	}
#endif

	if (m_externalMemory)
	{
		return m_mappedMemory[addr];
	}

	if (m_isBootromMapped && addr < BOOTROM_SIZE)
	{
		return m_bootrom[addr];
	}

	if (addr <= ROM_END)
	{
		return m_romMemory[m_mbc->GetROMAddr(addr)];
	}

	if (addr >= EXTERNAL_RAM_BEGIN && addr < EXTERNAL_RAM_BEGIN + RAM_BANK_SIZE)
	{
		return m_externalRamMemory[m_mbc->GetRAMAddr(addr)];
	}

	return m_mappedMemory[addr];
}

void Memory::Serialize(std::vector<Chunk>& chunks, std::vector<uint8_t>& data)
{
	uint32_t ramSize = m_mbc->GetRAMSize();
	uint32_t dataSize = MEMORY_SIZE + ramSize + sizeof(bool);
	uint8_t* rawData = CreateChunkAndGetDataPtr(chunks, data, dataSize, ChunkId::Memory);

	WriteAndMove(rawData, m_mappedMemory, MEMORY_SIZE);

	if (ramSize > 0)
	{
		WriteAndMove(rawData, m_externalRamMemory, ramSize);
	}

	WriteAndMove(rawData, &m_isBootromMapped, sizeof(bool));
}

void Memory::Deserialize(const Chunk* chunks, const uint32_t& chunkCount, const uint8_t* data, const uint32_t& dataSize)
{
	const Chunk* myChunk = FindChunk(chunks, chunkCount, ChunkId::Memory);
	if (myChunk == nullptr)
	{
		return;
	}

	data += myChunk->m_offset;

	ReadAndMove(data, m_mappedMemory, MEMORY_SIZE);

	uint32_t ramSize = m_mbc->GetRAMSize();
	if (ramSize > 0)
	{
		ReadAndMove(data, m_externalRamMemory, ramSize);
	}

	ReadAndMove(data, &m_isBootromMapped, sizeof(bool));
}