#pragma once
#include "../Include/Emulator.h"

#define ROM_END 0x7FFF
#define ROM_BANK_SIZE 0x4000
#define RAM_BANK_SIZE 0x2000
#define EXTERNAL_RAM_BEGIN 0xA000

class MemoryBankController
{
public:
	MemoryBankController();
	MemoryBankController(uint8_t headerType, uint8_t headerRomBanks, uint8_t headerRamBanks);

	bool WriteRegister(uint16_t addr, uint8_t value);
	uint32_t GetRAMAddr(uint16_t addr) const;
	uint32_t GetROMAddr(uint16_t addr) const;

	uint16_t GetRAMSize() const;

	struct Registers
	{
		Registers();
		bool m_isRAMEnabled;
		uint8_t m_primaryBankRegister;
		uint8_t m_secondaryBankRegister;
		uint8_t m_tertiaryBankRegister;
	};

private:
	enum class Type
	{
		None = 0,
		MBC1 = 1,
		MBC3 = 2,
		MBC5 = 3
	};

	Type GetTypeFromHeaderCode(uint8_t header) const;

	Registers m_registers;
	const Type m_type;
	const uint16_t m_romBankCount;
	const uint16_t m_ramBankCount;
};
