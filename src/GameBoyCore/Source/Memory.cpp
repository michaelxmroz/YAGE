#include "Memory.h"
#include "Clock.h"
#include "Logging.h"

#define ROM_END 0x7FFF
#define ROM_BANK_SIZE 0x4000
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

#define MBC_ROM_BANK_NR 0x2000

Memory::Memory()
{
	m_mappedMemory = new uint8_t[MEMORY_SIZE];
	m_externalMemory = false;
	Init();
}

Memory::Memory(uint8_t* rawMemory)
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
	}
	if (m_bootrom != nullptr)
	{
		delete[] m_bootrom;
	}
	if (m_romMemory != nullptr)
	{
		delete[] m_romMemory;
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
			MBCControl(addr, value);
			return;
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
		m_writeCallbacks[addr](this, addr, prevValue, value);
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

void Memory::MapROM(const char* rom, uint32_t size)
{
	m_romMemory = new uint8_t[size];
	memcpy(m_romMemory, rom, size);
#ifdef TRACK_UNINITIALIZED_MEMORY_READS
	memset(m_initializationTracker, 1, ROM_END + 1);
#endif
}

void Memory::MapBootrom(const char* rom, uint32_t size)
{
	m_bootrom = new uint8_t[BOOTROM_SIZE];
	memcpy(m_bootrom, rom, size);
	m_isBootromMapped = true;
}

void Memory::RegisterCallback(uint16_t addr, MemoryWriteCallback callback)
{
	m_writeCallbacks[addr] = callback;
}

void Memory::DeregisterCallback(uint16_t addr)
{
	m_writeCallbacks[addr] = nullptr;
}

void Memory::SetVRamAccess(VRamAccess access)
{
	m_vRamAccess = access;
}

void Memory::Init()
{
	m_romMemory = nullptr;
	m_isBootromMapped = false;
	m_bootrom = nullptr;
	m_selectedROMBankSlot1 = 1;

	m_writeCallbacks = new MemoryWriteCallback[MEMORY_SIZE];
	memset(m_writeCallbacks, 0, sizeof(MemoryWriteCallback) * MEMORY_SIZE);
	RegisterCallback(DMA_REGISTER, DoDMA);
	RegisterCallback(BOOTROM_BANK, UnmapBootrom);

	m_vRamAccess = VRamAccess::All;

#ifdef TRACK_UNINITIALIZED_MEMORY_READS
	m_initializationTracker = new uint8_t[MEMORY_SIZE];
	memset(m_initializationTracker, 0, MEMORY_SIZE);
#endif
}

void Memory::MBCControl(uint16_t addr, uint8_t value)
{
	if (addr >= MBC_ROM_BANK_NR)
	{
		m_selectedROMBankSlot1 = std::max(1, value & 0x1F);
	}
}

void Memory::DoDMA(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue)
{
	uint16_t source = static_cast<uint16_t>((*memory)[DMA_REGISTER]) << 8;
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

void Memory::UnmapBootrom(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue)
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
		if (addr >= ROM_BANK_SIZE)
		{
			uint32_t offset = (m_selectedROMBankSlot1 - 1) * ROM_BANK_SIZE;
			uint32_t adjustedAddr = addr + offset;
			return m_romMemory[adjustedAddr];
		}
		return m_romMemory[addr];
	}
	return m_mappedMemory[addr];
}
