#pragma once
#include <cstdint>

namespace EmulatorInputs
{
	enum class DPad
	{
		Right = 1,
		Left = 2,
		Up = 4,
		Down = 8
	};

	enum class Buttons
	{
		A = 1,
		B = 2,
		Select = 4,
		Start = 8
	};

	struct InputState
	{
		InputState() : m_dPad(0x0F), m_buttons(0x0F)
		{
		}
		uint8_t m_dPad;
		uint8_t m_buttons;

		void SetButtonDown(DPad pad)
		{
			m_dPad &= (~static_cast<uint8_t>(pad));
		}
		void SetButtonDown(Buttons button)
		{
			m_buttons &= (~static_cast<uint8_t>(button));
		}
	};
}

namespace EmulatorConstants
{
	const uint32_t SCREEN_WIDTH = 160;
	const uint32_t SCREEN_HEIGHT = 144;
	const uint32_t SCREEN_SIZE = SCREEN_WIDTH * SCREEN_HEIGHT;
	const float PREFERRED_REFRESH_RATE = 59.73f;
}

class Emulator
{
public:

	typedef void (*LoggerCallback)(const char* message, uint8_t severity);
	typedef void (*PersistentMemoryCallback)(const void* data, uint32_t size);

	static Emulator* Create();
	static void Delete(Emulator* emulator);

	virtual void SetLoggerCallback(LoggerCallback callback) = 0;
	virtual void Load(const char* rom, uint32_t size) = 0;
	virtual void Load(const char* rom, uint32_t size, const char* bootrom, uint32_t bootromSize) = 0;
	virtual void LoadPersistentMemory(const char* ram, uint32_t size) = 0;
	virtual void SetPersistentMemoryCallback(PersistentMemoryCallback callback) = 0;

	virtual void Step(EmulatorInputs::InputState) = 0;
	virtual const void* GetFrameBuffer() = 0;
protected:
	virtual ~Emulator();
};