#include "Memory.h"
#include "Clock.h"
#include "Logging.h"

#define ROM_END 0x7FFF
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

Memory::Memory()
{
	m_memory = new uint8_t[MEMORY_SIZE];
	m_externalMemory = false;

	Init();
}

Memory::Memory(uint8_t* rawMemory)
{
	m_memory = rawMemory;
	m_externalMemory = true;

	Init();
}

Memory::~Memory()
{
	if (!m_externalMemory)
	{
		delete[] m_memory;
	}
	delete[] m_writeCallbacks;
}

void Memory::Write(uint16_t addr, uint8_t value)
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
		LOG_ERROR(string_format("Trying to write ROM addr %x", addr).c_str());
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

	uint8_t prevValue = m_memory[addr];
	m_memory[addr] = value;

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
	m_memory[addr] = value;

#ifdef TRACK_UNINITIALIZED_MEMORY_READS
	m_initializationTracker[addr] = 1;
#endif
}

uint8_t Memory::ReadDirect(uint16_t addr)
{
	return m_memory[addr];
}

const SpriteAttributes& Memory::ReadOAMEntry(uint8_t index) const
{
	return reinterpret_cast<SpriteAttributes*>(m_memory + OAM_START)[index];
}

void Memory::ClearMemory()
{
	memset(m_memory, 0, MEMORY_SIZE);
#ifdef TRACK_UNINITIALIZED_MEMORY_READS
	memset(m_initializationTracker, 0, MEMORY_SIZE);
#endif

	WriteDirect(DMA_REGISTER, 0xFF);
}

void Memory::ClearVRAM()
{
	memset(m_memory + VRAM_START, 0, VRAM_END - VRAM_START);
#ifdef TRACK_UNINITIALIZED_MEMORY_READS
	memset(m_initializationTracker + VRAM_START, 1, VRAM_END - VRAM_START);
#endif
}

void Memory::MapROM(const char* rom, uint32_t size)
{
	memcpy(m_memory, rom, size);
#ifdef TRACK_UNINITIALIZED_MEMORY_READS
	memset(m_initializationTracker, 1, size);
#endif
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
	m_writeCallbacks = new MemoryWriteCallback[MEMORY_SIZE];
	memset(m_writeCallbacks, 0, sizeof(MemoryWriteCallback) * MEMORY_SIZE);
	RegisterCallback(DMA_REGISTER, DoDMA);

	m_vRamAccess = VRamAccess::All;

#ifdef TRACK_UNINITIALIZED_MEMORY_READS
	m_initializationTracker = new uint8_t[MEMORY_SIZE];
	memset(m_initializationTracker, 0, MEMORY_SIZE);
#endif
}

void Memory::DoDMA(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue)
{
	uint16_t source = static_cast<uint16_t>((*memory)[DMA_REGISTER]) << 8;
	memcpy(memory->m_memory + OAM_START, memory->m_memory + source, OAM_SIZE);

#ifdef TRACK_UNINITIALIZED_MEMORY_READS
	memset(memory->m_initializationTracker + OAM_START, 1, OAM_SIZE);
#endif
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

	return m_memory[addr];
}
