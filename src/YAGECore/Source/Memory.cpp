#include "Memory.h"
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
#define HRAM_BEGIN 0xFF80

#define BOOTROM_BANK 0xFF50
#define BOOTROM_SIZE 0x100

#define HEADER_CHECKSUM 0x014D

#define IO_REGISTERS_BEGIN 0xFF00
#define IO_REGISTERS_END 0xFF7F

#define DIVIDER_REGISTER 0xFF04

Memory::Memory(GamestateSerializer* serializer) : ISerializable(serializer)
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

	delete[] m_writeCallbacks;
}

void Memory::Write(uint16_t addr, uint8_t value)
{
	if (m_DMAInProgress && addr < IO_REGISTERS_BEGIN)
	{
		return;
	}
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
			m_mbc->WriteRegister(addr, value);
			return;
		}
		else if (addr >= EXTERNAL_RAM_BEGIN && addr < EXTERNAL_RAM_BEGIN + RAM_BANK_SIZE)
		{
			m_mbc->Write(addr, value);
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

	value = CheckForIOReadOnlyBitOverride(addr, value);

	WriteInternal(addr, value);
}

void Memory::WriteDirect(uint16_t addr, uint8_t value)
{
	m_mappedMemory[addr] = value;

#ifdef TRACK_UNINITIALIZED_MEMORY_READS
	m_initializationTracker[addr] = 1;
#endif
}

uint8_t Memory::ReadDirect(uint16_t addr) const
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

void Memory::ClearRange(uint16_t start, uint16_t end)
{
	memset(m_mappedMemory + start, 0, end - start);
}

void Memory::ClearVRAM()
{
	memset(m_mappedMemory + VRAM_START, 0, VRAM_END - VRAM_START);
#ifdef TRACK_UNINITIALIZED_MEMORY_READS
	memset(m_initializationTracker + VRAM_START, 1, VRAM_END - VRAM_START);
#endif
}

void Memory::MapROM(GamestateSerializer* serializer, const char* rom, uint32_t size)
{
	m_mbc = new MemoryBankController(serializer, rom, size);

#ifdef TRACK_UNINITIALIZED_MEMORY_READS
	memset(m_initializationTracker, 1, ROM_END + 1);
	memset(m_initializationTracker + EXTERNAL_RAM_BEGIN, 1, RAM_BANK_SIZE);
#endif
}

void Memory::DeserializePersistentData(const char* ram, uint32_t size)
{
	m_mbc->DeserializePersistentData(ram, size);
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

void Memory::RegisterRamSaveCallback(Emulator::PersistentMemoryCallback callback)
{
	m_mbc->RegisterRamSaveCallback(callback);
}

void Memory::SetVRamAccess(VRamAccess access)
{
	m_vRamAccess = access;
}

uint8_t Memory::GetHeaderChecksum() const
{
	return m_mbc->ReadROM(HEADER_CHECKSUM);
}

void Memory::Init()
{
	m_isBootromMapped = false;
	m_bootrom = nullptr;
	m_mbc = nullptr;

	m_DMAInProgress = false;
	m_DMAProgress = 0;

	memset(m_unusedIOBitsOverride, 0, IOPORTS_COUNT);
	memset(m_writeOnlyIOBitsOverride, 0, IOPORTS_COUNT);
	memset(m_readOnlyIOBitsOverride, 0, IOPORTS_COUNT);

	m_writeCallbacks = new MemoryWriteCallback[MEMORY_SIZE];
	memset(m_writeCallbacks, 0, sizeof(MemoryWriteCallback) * MEMORY_SIZE);

	m_callbackUserData = new uint64_t[MEMORY_SIZE];
	memset(m_callbackUserData, 0, sizeof(uint64_t) * MEMORY_SIZE);

	RegisterCallback(DMA_REGISTER, DoDMA, nullptr);
	RegisterCallback(BOOTROM_BANK, UnmapBootrom, nullptr);

	m_vRamAccess = VRamAccess::All;

	RegisterUnusedIORegisters();

#ifdef TRACK_UNINITIALIZED_MEMORY_READS
	m_initializationTracker = new uint8_t[MEMORY_SIZE];
	memset(m_initializationTracker, 0, MEMORY_SIZE);
	//skip initialization checks for APU wave ram
	memset(m_initializationTracker + 0xFF30, 1, 0xFF3F - 0xFF30 + 1);
#endif
}

void Memory::DoDMA(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue, void* userData)
{
	if (memory->m_DMAInProgress)
	{
		return;
	}
	uint16_t source = static_cast<uint16_t>((*memory)[DMA_REGISTER]) << 8;
	//TODO DMA from external ram?
	if (source < ROM_END && !memory->m_externalMemory)
	{
		memcpy(memory->m_mappedMemory + OAM_START, memory->m_mbc->GetROMMemoryOffset(source), OAM_SIZE);
	}
	else
	{
		memcpy(memory->m_mappedMemory + OAM_START, memory->m_mappedMemory + source, OAM_SIZE);
	}

	memory->m_DMAInProgress = true;

#ifdef TRACK_UNINITIALIZED_MEMORY_READS
	memset(memory->m_initializationTracker + OAM_START, 1, OAM_SIZE);
#endif
}

void Memory::UnmapBootrom(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue, void* userData)
{
	memory->m_isBootromMapped = false;
}

void Memory::Update()
{
	const uint32_t DMA_DURATION = 160;
	if (m_DMAInProgress)
	{
		m_DMAProgress++;
		if (m_DMAProgress == DMA_DURATION - 1)
		{
			m_DMAProgress = 0;
			m_DMAInProgress = false;
		}
	}
}

uint8_t Memory::operator[](uint16_t addr) const
{
	if (m_DMAInProgress && addr < IO_REGISTERS_BEGIN)
	{
		return 0xFF;
	}

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
		return m_mbc->ReadROM(addr);
	}

	if (addr >= EXTERNAL_RAM_BEGIN && addr < EXTERNAL_RAM_BEGIN + RAM_BANK_SIZE)
	{
		return m_mbc->ReadRAM(addr);
	}

	uint8_t memoryVal = m_mappedMemory[addr];


	memoryVal = CheckForIOUnusedBitOverride(addr, memoryVal);
	memoryVal = CheckForIOWriteOnlyBitOverride(addr, memoryVal);

	return memoryVal;
}

//IO Register read function that skips unused bits and write only overrides
uint8_t Memory::ReadIO(uint16_t addr) const
{
	if (addr < IO_REGISTERS_BEGIN || addr > IO_REGISTERS_END)
	{		
		LOG_ERROR(string_format("Trying to read IO registers at invalid address %x", addr).c_str());
		return 0;
	}

	return m_mappedMemory[addr];
}

//IO Register write function that skips read only overrides
void Memory::WriteIO(uint16_t addr, uint8_t value)
{
	if (addr < IO_REGISTERS_BEGIN || addr > IO_REGISTERS_END)
	{
		LOG_ERROR(string_format("Trying to write IO registers at invalid address %x", addr).c_str());
		return;
	}

	WriteInternal(addr, value);
}

void Memory::Serialize(std::vector<Chunk>& chunks, std::vector<uint8_t>& data)
{
	uint32_t dataSize = MEMORY_SIZE + sizeof(bool) + sizeof(bool) + sizeof(uint32_t);
	uint8_t* rawData = CreateChunkAndGetDataPtr(chunks, data, dataSize, ChunkId::Memory);

	WriteAndMove(rawData, m_mappedMemory, MEMORY_SIZE);

	WriteAndMove(rawData, &m_isBootromMapped, sizeof(bool));
	WriteAndMove(rawData, &m_DMAInProgress, sizeof(bool));
	WriteAndMove(rawData, &m_DMAProgress, sizeof(uint32_t));
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

	ReadAndMove(data, &m_isBootromMapped, sizeof(bool));
	ReadAndMove(data, &m_DMAInProgress, sizeof(bool));
	ReadAndMove(data, &m_DMAProgress, sizeof(uint32_t));
}

inline void Memory::WriteInternal(uint16_t addr, uint8_t value)
{
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

uint8_t Memory::CheckForIOUnusedBitOverride(uint16_t addr, uint8_t readValue) const
{
	//Some IO registers have unused bits, these bits should always return 1 when read.
	// Each subsystem needs to register its own unused bit overrides
	if (addr >= IO_REGISTERS_BEGIN && addr <= IO_REGISTERS_END)
	{
		readValue |= m_unusedIOBitsOverride[(addr & 0xFF)];
	}
	return readValue;
}

uint8_t Memory::CheckForIOWriteOnlyBitOverride(uint16_t addr, uint8_t readValue) const
{
	//Some IO registers are write only, these bits should always return 1 when read.
	if (addr >= IO_REGISTERS_BEGIN && addr <= IO_REGISTERS_END)
	{
		readValue |= m_writeOnlyIOBitsOverride[(addr & 0xFF)];
	}
	return readValue;
}

uint8_t Memory::CheckForIOReadOnlyBitOverride(uint16_t addr, uint8_t writeValue) const
{
	//Some IO registers are read only, these bits cannot be written to by the CPU.
	if (addr >= IO_REGISTERS_BEGIN && addr <= IO_REGISTERS_END)
	{
		uint8_t readOnlyBits = m_readOnlyIOBitsOverride[(addr & 0xFF)];
		writeValue = (writeValue & ~readOnlyBits) | (m_mappedMemory[addr] & readOnlyBits);
	}
	return writeValue;
}

void Memory::RegisterUnusedIORegisters()
{
	//Unused IO registers
	AddIOUnusedBitsOverride(0xFF03, 0b11111111);
	AddIOUnusedBitsOverride(0xFF08, 0b11111111);
	AddIOUnusedBitsOverride(0xFF09, 0b11111111);
	AddIOUnusedBitsOverride(0xFF0A, 0b11111111);
	AddIOUnusedBitsOverride(0xFF0B, 0b11111111);
	AddIOUnusedBitsOverride(0xFF0C, 0b11111111);
	AddIOUnusedBitsOverride(0xFF0D, 0b11111111);
	AddIOUnusedBitsOverride(0xFF0E, 0b11111111);
	AddIOUnusedBitsOverride(0xFF0F, 0b11111111);
	AddIOUnusedBitsOverride(0xFF27, 0b11111111);
	AddIOUnusedBitsOverride(0xFF28, 0b11111111);
	AddIOUnusedBitsOverride(0xFF29, 0b11111111);
	AddIOUnusedBitsOverride(0xFF2A, 0b11111111);
	AddIOUnusedBitsOverride(0xFF2B, 0b11111111);
	AddIOUnusedBitsOverride(0xFF2C, 0b11111111);
	AddIOUnusedBitsOverride(0xFF2D, 0b11111111);
	AddIOUnusedBitsOverride(0xFF2E, 0b11111111);
	AddIOUnusedBitsOverride(0xFF2F, 0b11111111);
	AddIOUnusedBitsOverride(0xFF4C, 0b11111111);
	AddIOUnusedBitsOverride(0xFF4D, 0b11111111);
	AddIOUnusedBitsOverride(0xFF4E, 0b11111111);
	AddIOUnusedBitsOverride(0xFF50, 0b11111111);
	AddIOUnusedBitsOverride(0xFF56, 0b11111111);
	AddIOUnusedBitsOverride(0xFF57, 0b11111111);
	AddIOUnusedBitsOverride(0xFF58, 0b11111111);
	AddIOUnusedBitsOverride(0xFF59, 0b11111111);
	AddIOUnusedBitsOverride(0xFF5A, 0b11111111);
	AddIOUnusedBitsOverride(0xFF5B, 0b11111111);
	AddIOUnusedBitsOverride(0xFF5C, 0b11111111);
	AddIOUnusedBitsOverride(0xFF5D, 0b11111111);
	AddIOUnusedBitsOverride(0xFF5E, 0b11111111);
	AddIOUnusedBitsOverride(0xFF5F, 0b11111111);
	AddIOUnusedBitsOverride(0xFF60, 0b11111111);
	AddIOUnusedBitsOverride(0xFF61, 0b11111111);
	AddIOUnusedBitsOverride(0xFF62, 0b11111111);
	AddIOUnusedBitsOverride(0xFF63, 0b11111111);
	AddIOUnusedBitsOverride(0xFF64, 0b11111111);
	AddIOUnusedBitsOverride(0xFF65, 0b11111111);
	AddIOUnusedBitsOverride(0xFF66, 0b11111111);
	AddIOUnusedBitsOverride(0xFF67, 0b11111111);
	AddIOUnusedBitsOverride(0xFF6C, 0b11111111);
	AddIOUnusedBitsOverride(0xFF6D, 0b11111111);
	AddIOUnusedBitsOverride(0xFF6E, 0b11111111);
	AddIOUnusedBitsOverride(0xFF6F, 0b11111111);
	AddIOUnusedBitsOverride(0xFF71, 0b11111111);
	AddIOUnusedBitsOverride(0xFF72, 0b11111111);
	AddIOUnusedBitsOverride(0xFF73, 0b11111111);
	AddIOUnusedBitsOverride(0xFF74, 0b11111111);
	AddIOUnusedBitsOverride(0xFF75, 0b11111111);
	AddIOUnusedBitsOverride(0xFF76, 0b11111111);
	AddIOUnusedBitsOverride(0xFF77, 0b11111111);
	AddIOUnusedBitsOverride(0xFF78, 0b11111111);
	AddIOUnusedBitsOverride(0xFF79, 0b11111111);
	AddIOUnusedBitsOverride(0xFF7A, 0b11111111);
	AddIOUnusedBitsOverride(0xFF7B, 0b11111111);
	AddIOUnusedBitsOverride(0xFF7C, 0b11111111);
	AddIOUnusedBitsOverride(0xFF7D, 0b11111111);
	AddIOUnusedBitsOverride(0xFF7E, 0b11111111);
	AddIOUnusedBitsOverride(0xFF7F, 0b11111111);
	//GBC only
	AddIOUnusedBitsOverride(0xFF4F, 0b11111111);
	AddIOUnusedBitsOverride(0xFF51, 0b11111111);
	AddIOUnusedBitsOverride(0xFF52, 0b11111111);
	AddIOUnusedBitsOverride(0xFF53, 0b11111111);
	AddIOUnusedBitsOverride(0xFF54, 0b11111111);
	AddIOUnusedBitsOverride(0xFF55, 0b11111111);
	AddIOUnusedBitsOverride(0xFF68, 0b11111111);
	AddIOUnusedBitsOverride(0xFF69, 0b11111111);
	AddIOUnusedBitsOverride(0xFF6A, 0b11111111);
	AddIOUnusedBitsOverride(0xFF6B, 0b11111111);
	AddIOUnusedBitsOverride(0xFF70, 0b11111111);
}

void Memory::AddIOUnusedBitsOverride(uint16_t addr, uint8_t mask)
{
	uint32_t index = addr & 0xFF;
	m_unusedIOBitsOverride[index] = mask;
}

void Memory::AddIOReadOnlyBitsOverride(uint16_t addr, uint8_t mask)
{
	uint32_t index = addr & 0xFF;
	m_readOnlyIOBitsOverride[index] = mask;
}

void Memory::AddIOWriteOnlyBitsOverride(uint16_t addr, uint8_t mask)
{
	uint32_t index = addr & 0xFF;
	m_writeOnlyIOBitsOverride[index] = mask;
}

void Memory::AddIOReadOnlyRange(uint16_t start, uint16_t end)
{
	for (uint16_t i = start; i <= end; ++i)
	{
		AddIOReadOnlyBitsOverride(i, 0xFF);
	}
}

void Memory::RemoveIOReadOnlyRange(uint16_t start, uint16_t end)
{
	for (uint16_t i = start; i <= end; ++i)
	{
		AddIOReadOnlyBitsOverride(i, 0x00);
	}
}

void Memory::AddIOWriteOnlyRange(uint16_t start, uint16_t end)
{
	for (uint16_t i = start; i <= end; ++i)
	{
		AddIOWriteOnlyBitsOverride(i, 0xFF);
	}
}
