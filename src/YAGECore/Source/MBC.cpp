#include "MBC.h"
#include "Helpers.h"
#include "Logging.h"
#include "Allocator.h"

#define HEADER_CARTRIDGE_TYPE 0x0147
#define HEADER_ROM_SIZE 0x0148
#define HEADER_RAM_SIZE 0x0149
#define HEADER_CHECKSUM 0x014D
#define HEADER_ROM_NAME_BEGIN 0x0134
#define HEADER_ROM_NAME_LENGTH 0x10

#define MBC_ROM_BANKING_REGISTER 0x2000
#define MBC_ROM_BANKING_SECONDARY_REGISTER 0x3000
#define MBC_SECONDARY_BANK_REGISTER 0x4000
#define MBC_ROM_BANK_MODE_SELECT_REGISTER 0x6000
#define MBC_LARGE_ROM 32

#define EXTERNAL_RAM_ENABLE_VALUE 0x0A

#define RTC_REGISTER_SELECT_VALUE 0x08

#define PERSISTENT_DATA_NAME "GameboySavegame"
#define PERSISTENT_DATA_VERSION 1

namespace MBC_Internal
{
	namespace MBC1
	{
		FORCE_INLINE bool WriteRegister(uint16_t addr, uint8_t value, MemoryBankController::Registers& registers)
		{
			if (addr >= MBC_ROM_BANK_MODE_SELECT_REGISTER)
			{
				registers.m_tertiaryBankRegister = value > 0 ? 1 : 0;
			}
			else if (addr >= MBC_SECONDARY_BANK_REGISTER)
			{
				registers.m_secondaryBankRegister = value & 0x03;
			}
			else if (addr >= MBC_ROM_BANKING_REGISTER)
			{
				registers.m_primaryBankRegister = std::max(1, value & 0x1F);
			}
			else
			{
				registers.m_isRAMEnabled = value == EXTERNAL_RAM_ENABLE_VALUE;
				return !registers.m_isRAMEnabled;
			}
			return false;
		}


		FORCE_INLINE uint32_t GetROMAddr(uint16_t addr, const MemoryBankController::Registers& registers, uint16_t romBankCount)
		{
			uint32_t bankId = 0;
			if (addr >= ROM_BANK_SIZE)
			{
				bankId = registers.m_primaryBankRegister - 1;
				if (romBankCount > MBC_LARGE_ROM && registers.m_tertiaryBankRegister == 0)
				{
					bankId += (registers.m_secondaryBankRegister << 5);
				}
			}
			else if (romBankCount > MBC_LARGE_ROM && registers.m_tertiaryBankRegister > 0)
			{
				bankId = (registers.m_secondaryBankRegister << 5);
			}

			int32_t offset = bankId * ROM_BANK_SIZE;
			uint32_t adjustedAddr = addr + offset;
			return adjustedAddr;
		}

		FORCE_INLINE uint32_t GetRAMAddr(uint16_t addr, const MemoryBankController::Registers& registers)
		{
			uint32_t bankId = 0;
			if (registers.m_tertiaryBankRegister > 0)
			{
				bankId = registers.m_secondaryBankRegister;
			}
			uint32_t offset = bankId * RAM_BANK_SIZE;
			return addr - EXTERNAL_RAM_BEGIN + offset;
		}
	}

	namespace MBC3
	{
		bool HasRTC(uint8_t typeCode)
		{
			switch (typeCode)
			{
			case 0x0F:
			case 0x10:
				return true;
			default:
				return false;
			}
		}

		FORCE_INLINE bool WriteRegister(uint16_t addr, uint8_t value, MemoryBankController::Registers& registers, bool hasRTC)
		{
			if (addr >= MBC_ROM_BANK_MODE_SELECT_REGISTER && hasRTC && registers.m_isRAMEnabled)
			{
				registers.m_RTC.m_isLatched = value == 0x01;
			}
			else if (addr >= MBC_SECONDARY_BANK_REGISTER)
			{
				if (hasRTC && value >= RTC_REGISTER_SELECT_VALUE && registers.m_isRAMEnabled)
				{
					registers.m_RTC.m_selectedReg = value - RTC_REGISTER_SELECT_VALUE;
				}
				else if (value <= 0x03)
				{
					registers.m_secondaryBankRegister = value & 0x03;
				}
			}
			else if (addr >= MBC_ROM_BANKING_REGISTER)
			{
				registers.m_primaryBankRegister = std::max(1, value & 0x7F);
			}
			else
			{
				registers.m_isRAMEnabled = value == EXTERNAL_RAM_ENABLE_VALUE;
				return !registers.m_isRAMEnabled;
			}
			return false;
		}


		FORCE_INLINE uint32_t GetROMAddr(uint16_t addr, const MemoryBankController::Registers& registers, uint16_t romBankCount)
		{
			if (addr < ROM_BANK_SIZE)
			{
				return addr;
			}

			uint32_t bankId = registers.m_primaryBankRegister;
			return (addr & (ROM_BANK_SIZE - 1)) + bankId * ROM_BANK_SIZE;
		}

		FORCE_INLINE uint32_t GetRAMAddr(uint16_t addr, const MemoryBankController::Registers& registers)
		{
			uint32_t bankId = registers.m_secondaryBankRegister;
			uint32_t offset = bankId * RAM_BANK_SIZE;
			return addr - EXTERNAL_RAM_BEGIN + offset;
		}
	}

	namespace MBC5
	{
		FORCE_INLINE bool WriteRegister(uint16_t addr, uint8_t value, MemoryBankController::Registers& registers)
		{
			if (addr >= MBC_SECONDARY_BANK_REGISTER)
			{
				registers.m_secondaryBankRegister = value & 0x0F;
			}
			else if (addr >= MBC_ROM_BANKING_SECONDARY_REGISTER)
			{
				registers.m_tertiaryBankRegister = value & 0x01;
			}
			else if (addr >= MBC_ROM_BANKING_REGISTER)
			{
				registers.m_primaryBankRegister = value;
			}
			else
			{
				registers.m_isRAMEnabled = value == EXTERNAL_RAM_ENABLE_VALUE;
				return !registers.m_isRAMEnabled;
			}
			return false;
		}

		FORCE_INLINE uint32_t GetROMAddr(uint16_t addr, const MemoryBankController::Registers& registers, uint16_t romBankCount)
		{
			if (addr < ROM_BANK_SIZE)
			{
				return addr;
			}
			uint32_t bankId = registers.m_primaryBankRegister + (registers.m_tertiaryBankRegister << 9);
			return (addr & (ROM_BANK_SIZE - 1)) + bankId * ROM_BANK_SIZE;
		}

		FORCE_INLINE uint32_t GetRAMAddr(uint16_t addr, const MemoryBankController::Registers& registers)
		{
			return MBC3::GetRAMAddr(addr, registers);
		}
	}

	uint16_t GetRAMBankCountFromHeader(uint8_t headerRamBanks)
	{
		switch (headerRamBanks)
		{
		case 0:
		case 1:
			return 0;
		case 2:
			return 1;
		case 3:
			return 4;
		case 4:
			return 16;
		case 5:
			return 8;
		default:
			return 0;
		}
	}
}

MemoryBankController::MemoryBankController()
	: ISerializable(nullptr)
	, m_type(Type::None)
	, m_hasRTC(false)
	, m_registers()
	, m_romBankCount(0)
	, m_ramBankCount(0)
	, m_rom(nullptr)
	, m_ram(nullptr)
	, m_onRamSave(nullptr)
{
}

MemoryBankController::MemoryBankController(GamestateSerializer* serializer, const char* rom, uint32_t size)
	: ISerializable(serializer)
	, m_type(GetTypeFromHeaderCode(rom[HEADER_CARTRIDGE_TYPE]))
	, m_hasRTC(m_type == Type::MBC3 && MBC_Internal::MBC3::HasRTC(rom[HEADER_CARTRIDGE_TYPE]))
	, m_registers()
	, m_romBankCount(static_cast<uint16_t>(pow(2, rom[HEADER_ROM_SIZE] + 1)))
	, m_ramBankCount(MBC_Internal::GetRAMBankCountFromHeader(rom[HEADER_RAM_SIZE]))
	, m_onRamSave(nullptr)
{

	if (size <= HEADER_RAM_SIZE)
	{
		LOG_ERROR("ROM size is too small to be a proper ROM file");
		return;
	}
	m_rom = Y_NEW_A(uint8_t,size);
	memcpy(m_rom, rom, size);

	uint32_t ramSize = GetRAMSize();
	m_ram = Y_NEW_A(uint8_t, ramSize);
	memset(m_ram, 0, ramSize);

}

MemoryBankController::~MemoryBankController()
{
	Y_DELETE_A(m_rom);
	Y_DELETE_A(m_ram);
}

void MemoryBankController::WriteRegister(uint16_t addr, uint8_t value)
{
	bool previousRamEnable = m_registers.m_isRAMEnabled;

	switch (m_type)
	{
	case Type::None:
		break;
	case Type::MBC1:
		MBC_Internal::MBC1::WriteRegister(addr, value, m_registers);
	case Type::MBC3:
		MBC_Internal::MBC3::WriteRegister(addr, value, m_registers, m_hasRTC);
	case Type::MBC5:
		MBC_Internal::MBC5::WriteRegister(addr, value, m_registers);
	}
	
	if (m_ram != nullptr && !m_registers.m_isRAMEnabled && previousRamEnable)
	{
		SerializePersistentData();	
	}
}

void MemoryBankController::Write(uint16_t addr, uint8_t value)
{
	if (m_ram != nullptr)
	{
		m_ram[GetRAMAddr(addr)] = value;
	}
	else
	{
		LOG_WARNING(string_format("Trying to write to non-existent RAM addr %x", addr).c_str());
	}

	//if(m_type == Type::MBC3 && m_hasRTC && m_registers.m_RTC.m_selectedReg)
	//return false;
}

uint8_t MemoryBankController::ReadRAM(uint16_t addr)
{
	if (m_ram == nullptr)
	{
		return 0xFF;
	}

	return m_ram[GetRAMAddr(addr)];
}

uint8_t MemoryBankController::ReadROM(uint16_t addr)
{
	return m_rom[GetROMAddr(addr)];
}

uint8_t* MemoryBankController::GetROMMemoryOffset(uint16_t addr)
{
	return m_rom + GetROMAddr(addr);
}

void MemoryBankController::RegisterRamSaveCallback(Emulator::PersistentMemoryCallback callback)
{
	m_onRamSave = callback;
}

uint32_t MemoryBankController::GetRAMAddr(uint16_t addr) const
{
	if (m_ramBankCount == 0 || !m_registers.m_isRAMEnabled)
	{
		LOG_ERROR("Trying to access invalid external ram address");
		return addr;
	}

	switch (m_type)
	{
	case Type::None:
		return addr - EXTERNAL_RAM_BEGIN;
	case Type::MBC1:
		return MBC_Internal::MBC1::GetRAMAddr(addr, m_registers);
	case Type::MBC3:
		return MBC_Internal::MBC3::GetRAMAddr(addr, m_registers);
	case Type::MBC5:
		return MBC_Internal::MBC5::GetRAMAddr(addr, m_registers);
	}
	
	return addr;
}

uint32_t MemoryBankController::GetROMAddr(uint16_t addr) const
{
	switch (m_type)
	{
	case Type::None:
		return addr;
	case Type::MBC1:
		return MBC_Internal::MBC1::GetROMAddr(addr, m_registers, m_romBankCount);
	case Type::MBC3:
		return MBC_Internal::MBC3::GetROMAddr(addr, m_registers, m_romBankCount);
	case Type::MBC5:
		return MBC_Internal::MBC5::GetROMAddr(addr, m_registers, m_romBankCount);
	}

	return addr;
}

uint16_t MemoryBankController::GetRAMSize() const
{
	return std::max(static_cast<uint16_t>(1),m_ramBankCount) * RAM_BANK_SIZE;
}

MemoryBankController::Type MemoryBankController::GetTypeFromHeaderCode(uint8_t header) const
{
	switch (header)
	{
	case 0x00:
		return Type::None;
	case 0x01:
	case 0x02:
	case 0x03:
		return Type::MBC1;
	case 0x0F:
	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13:
		return Type::MBC3;
	case 0x19:
	case 0x1A:
	case 0x1B:
	case 0x1C:
	case 0x1D:
	case 0x1E:
		return Type::MBC5;
	default:
		return Type::None;
	}
}

void MemoryBankController::SerializePersistentData()
{
	if (m_onRamSave == nullptr)
	{
		return;
	}

	SerializationParameters params;
	memcpy(params.m_dataName, PERSISTENT_DATA_NAME, strlen(PERSISTENT_DATA_NAME) + 1);
	params.m_version = PERSISTENT_DATA_VERSION;
	params.m_romChecksum = m_rom[HEADER_CHECKSUM];
	params.m_noHeaders = false;
	params.m_romName = std::string(reinterpret_cast<char*>(m_rom + HEADER_ROM_NAME_BEGIN));

	SerializationFactory serializer(params);
;
	
	uint32_t ramSize = GetRAMSize();
	uint32_t dataSize = ramSize;
	if (m_hasRTC)
	{
		dataSize += sizeof(RTC);
	}

	uint8_t* rawData = serializer.CreateChunk(ChunkId::MBC_Save, dataSize);

	WriteAndMove(rawData, m_ram, ramSize);

	if (m_hasRTC)
	{
		WriteAndMove(rawData, &m_registers.m_RTC, sizeof(RTC));
	}

	std::vector<uint8_t> buffer;
	
	serializer.Finish(buffer);

	m_onRamSave(buffer.data(), static_cast<uint32_t>(buffer.size()));
}

void MemoryBankController::DeserializePersistentData(const char* data, uint32_t size)
{
	SerializationParameters params;
	memcpy(params.m_dataName, PERSISTENT_DATA_NAME, strlen(PERSISTENT_DATA_NAME));
	params.m_version = PERSISTENT_DATA_VERSION;
	params.m_romChecksum = m_rom[HEADER_CHECKSUM];

	DeserializationFactory deserializer(params, reinterpret_cast<const uint8_t*>(data), size);

	DeserializationFactory::RawBuffers buffers = deserializer.GetRawBuffers(reinterpret_cast<const uint8_t*>(data));


	const Chunk* myChunk = FindChunk(buffers.m_chunks, buffers.m_chunkCount, ChunkId::MBC_Save);
	if (myChunk == nullptr)
	{
		return;
	}

	data += myChunk->m_offset;

	uint32_t ramSize = GetRAMSize();
	ReadAndMove(buffers.m_data, m_ram, ramSize);

	if (m_hasRTC)
	{
		ReadAndMove(buffers.m_data, &m_registers.m_RTC, sizeof(RTC));
	}

	deserializer.Finish();
}

void MemoryBankController::Serialize(std::vector<Chunk>& chunks, std::vector<uint8_t>& data)
{
	uint32_t ramSize = GetRAMSize();
	uint32_t dataSize = sizeof(Registers) + ramSize;
	uint8_t* rawData = CreateChunkAndGetDataPtr(chunks, data, dataSize, ChunkId::MBC);

	WriteAndMove(rawData, &m_registers, sizeof(Registers));

	WriteAndMove(rawData, m_ram, ramSize);
}

void MemoryBankController::Deserialize(const Chunk* chunks, const uint32_t& chunkCount, const uint8_t* data, const uint32_t& dataSize)
{
	const Chunk* myChunk = FindChunk(chunks, chunkCount, ChunkId::MBC);
	if (myChunk == nullptr)
	{
		return;
	}

	data += myChunk->m_offset;

	ReadAndMove(data, &m_registers, sizeof(Registers));

	uint32_t ramSize = GetRAMSize();
	ReadAndMove(data, m_ram, ramSize);

}

MemoryBankController::Registers::Registers()
	: m_primaryBankRegister(1)
	, m_secondaryBankRegister(0)
	, m_tertiaryBankRegister(0)
	, m_isRAMEnabled(false)
	, m_RTC()
{
}
