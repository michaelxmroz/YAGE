#include "MBC.h"
#include "Helpers.h"
#include "Logging.h"


#define MBC_ROM_BANKING_REGISTER 0x2000
#define MBC_ROM_BANKING_SECONDARY_REGISTER 0x3000
#define MBC_SECONDARY_BANK_REGISTER 0x4000
#define MBC_ROM_BANK_MODE_SELECT_REGISTER 0x6000
#define MBC_LARGE_ROM 32

#define EXTERNAL_RAM_ENABLE_VALUE 0x0A

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
		FORCE_INLINE bool WriteRegister(uint16_t addr, uint8_t value, MemoryBankController::Registers& registers)
		{
			if (addr >= MBC_ROM_BANK_MODE_SELECT_REGISTER)
			{
				//TODO RTC support
			}
			else if (addr >= MBC_SECONDARY_BANK_REGISTER)
			{
				registers.m_secondaryBankRegister = value & 0x03;
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
	, m_registers()
	, m_romBankCount(0)
	, m_ramBankCount(0)
{
}

MemoryBankController::MemoryBankController(Serializer* serializer, uint8_t headerType, uint8_t headerRomBanks, uint8_t headerRamBanks)
	: ISerializable(serializer)
	, m_type(GetTypeFromHeaderCode(headerType))
	, m_registers()
	, m_romBankCount(static_cast<uint16_t>(pow(2, headerRomBanks + 1)))
	, m_ramBankCount(MBC_Internal::GetRAMBankCountFromHeader(headerRamBanks))
{
}

bool MemoryBankController::WriteRegister(uint16_t addr, uint8_t value)
{
	switch (m_type)
	{
	case Type::None:
		break;
	case Type::MBC1:
		return MBC_Internal::MBC1::WriteRegister(addr, value, m_registers);
	case Type::MBC3:
		return MBC_Internal::MBC3::WriteRegister(addr, value, m_registers);
	case Type::MBC5:
		return MBC_Internal::MBC5::WriteRegister(addr, value, m_registers);
	}
	
	return false;
}

uint32_t MemoryBankController::GetRAMAddr(uint16_t addr) const
{
	if (m_ramBankCount == 0 || !m_registers.m_isRAMEnabled)
	{
		LOG_ERROR("Trying to access invalid external ram address");
		return 0x00;
	}

	switch (m_type)
	{
	case Type::None:
		return addr;
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
	return m_ramBankCount * RAM_BANK_SIZE;
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

void MemoryBankController::Serialize(std::vector<Chunk>& chunks, std::vector<uint8_t>& data)
{
	uint32_t dataSize = sizeof(Registers);
	uint8_t* rawData = CreateChunkAndGetDataPtr(chunks, data, dataSize, ChunkId::MBC);

	WriteAndMove(rawData, &m_registers, sizeof(Registers));
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
}

MemoryBankController::Registers::Registers()
	: m_primaryBankRegister(1)
	, m_secondaryBankRegister(0)
	, m_tertiaryBankRegister(0)
	, m_isRAMEnabled(false)
{
}
