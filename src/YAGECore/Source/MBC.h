#pragma once
#include "../Include/Emulator.h"
#include "Serialization.h"
#include "CppIncludes.h"

#define ROM_END 0x7FFF
#define ROM_BANK_SIZE 0x4000
#define RAM_BANK_SIZE 0x2000
#define EXTERNAL_RAM_BEGIN 0xA000

class MemoryBankController : ISerializable
{
public:
	MemoryBankController();
	MemoryBankController(GamestateSerializer* serializer, const char* rom, uint32_t size);
	virtual ~MemoryBankController();

	MemoryBankController(const MemoryBankController&) = delete;
	MemoryBankController& operator=(const MemoryBankController&) = delete;
	MemoryBankController(MemoryBankController&&) = delete;
	MemoryBankController& operator=(MemoryBankController&&) = delete;

	void WriteRegister(uint16_t addr, uint8_t value);
	void Write(uint16_t addr, uint8_t value);

	uint8_t ReadRAM(uint16_t addr);
	uint8_t ReadROM(uint16_t addr);

	void DeserializePersistentData(const char* ram, uint32_t size);

	uint8_t* GetROMMemoryOffset(uint16_t addr);

	void RegisterRamSaveCallback(Emulator::PersistentMemoryCallback callback);

	struct RTC
	{
		long long m_lastUpdate;
		bool m_isLatched;
		uint8_t m_selectedReg;
		uint8_t m_secReg;
		uint8_t m_minReg;
		uint8_t m_hourReg;
		uint8_t m_dayReg;
		uint8_t m_ctrlReg;
	};

	struct Registers
	{
		Registers();
		bool m_isRAMEnabled;
		uint8_t m_primaryBankRegister;
		uint8_t m_secondaryBankRegister;
		uint8_t m_tertiaryBankRegister;
		RTC m_RTC;
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

	uint32_t GetRAMAddr(uint16_t addr) const;
	uint32_t GetROMAddr(uint16_t addr) const;

	uint16_t GetRAMSize() const;

	void SerializePersistentData();

	void Serialize(uint8_t* data) override;
	void Deserialize(const uint8_t* data) override;
	virtual uint32_t GetSerializationSize() override;

	uint8_t* m_ram;
	uint8_t* m_rom;
	Registers m_registers;
	uint32_t m_currentlySelectedRTCReg;

	Emulator::PersistentMemoryCallback m_onRamSave;

	const Type m_type;
	const bool m_hasRTC;
	const uint16_t m_romBankCount;
	const uint16_t m_ramBankCount;

	yVector<uint8_t> m_persistentDataSerializationBuffer;
};
