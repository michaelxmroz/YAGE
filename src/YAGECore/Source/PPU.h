#pragma once
#include "Memory.h"
#include "PixelFIFO.h"
#include "../Include/Emulator.h"
#include "PixelFetcher.h"

#define MAX_SPRITES_PER_LINE 10

typedef void (*RenderFunc)(const void* image);

struct RGBA
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;

	bool operator==(const RGBA& other)
	{
		return r == other.r && g == other.g && b == other.b && a == other.a;
	}
};

class PPU : ISerializable
{
public:
	PPU(GamestateSerializer* serializer);

	~PPU() override;

	PPU(const PPU& other) = delete;
	PPU operator= (const PPU& other) = delete;

	void Init(Memory& memory);
	void Render(uint32_t mCycles, Memory& memory);
	void SwapBackbuffer();
	const void* GetFrameBuffer() const;

#if defined(_DEBUG)
	Emulator::FIFOSizes GetFIFOSizes() const;
	Emulator::PPUState GetPPUState();
#endif

private:

	enum class PPUState
	{
		HBlank = 0,
		VBlank = 1,
		OAMScan = 2,
		Drawing = 3
	};

	enum class WindowState
	{
		NoWindow = 0,
		InScanline = 1,
		Draw = 2
	};

	enum class StateTransition
	{
		None = 0,
		Cycle0 = 1,
		Cycle1 = 2,
		Cycle2 = 3,
	};

	void ScanOAM(uint32_t positionInLine, Memory& memory);
	void RenderNextPixel(Memory& memory);

	void TransitionToVBlank(Memory& memory, uint32_t processedCycles);
	void TransitionToHBlank(Memory& memory, uint32_t processedCycles);
	void TransitionToDraw(Memory& memory, uint32_t processedCycles);
	void TransitionToOAMScan(Memory& memory, uint32_t processedCycles);

	void DisableScreen(Memory& memory);
	void DrawPixels(Memory& memory, uint32_t& processedCycles);
	void CheckForInterrupts(Memory& memory);

	bool GetCurrentSprite(uint8_t& spriteIndex, uint8_t offset) const;

	static void CacheBackgroundPalette(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue, void* userData);
	static void LCDCWrite(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue, void* userData);

	struct TrackedBool
	{
		bool m_previous;
		bool m_current;

		void Reset();

		void Add(bool val);

		bool ShouldTrigger();
	};

	struct PPUData
	{
		PPUData()
			: m_totalCycles(0)
			, m_lineY(0)
			, m_lineSpriteCount(0)
			, m_frameCount(0)
			, m_backgroundFetcher(false)
			, m_spriteFetcher(true)
			, m_lineSprites()
			, m_lineX(0)
			, m_windowState(WindowState::NoWindow)
			, m_state(PPUState::OAMScan)
			, m_spritePrefetchLine(0)
			, m_lineSpriteMask(0)
			, m_cycleDebt(0)
			, m_cachedBackgroundColors()
			, m_fineScrollX(0)
			, m_windowLineY(0)
			, m_statLine()
			, m_vblankLine()
			, m_cachedBackgroundEnabled()
			, m_firstFrame(true)
			, m_stateTransition(StateTransition::None)
			, m_cyclesInMode(0)
		{}

		uint32_t m_totalCycles;
		int32_t m_cyclesInMode;
		int32_t m_cycleDebt;
		uint8_t m_lineY;
		uint8_t m_lineX;
		uint8_t m_lineSpriteCount;
		SpriteAttributes m_lineSprites[MAX_SPRITES_PER_LINE];
		uint16_t m_lineSpriteMask;
		uint8_t m_spritePrefetchLine;

		PixelFIFO m_spriteFIFO;
		PixelFIFO m_backgroundFIFO;

		PixelFetcher m_backgroundFetcher;
		PixelFetcher m_spriteFetcher;

		WindowState m_windowState;
		uint8_t m_windowLineY;

		uint32_t m_frameCount;

		PPUState m_state;
		StateTransition m_stateTransition;

		uint8_t m_fineScrollX;
		TrackedBool m_statLine;
		TrackedBool m_vblankLine;

		bool m_cachedBackgroundEnabled;
		RGBA m_cachedBackgroundColors[4];

		bool m_firstFrame;
	} data;
	RGBA* m_activeFrame;
	RGBA* m_backBuffer;


	// Inherited via ISerializable
	void Serialize(uint8_t* data) override;
	void Deserialize(const uint8_t* data) override;
	virtual uint32_t GetSerializationSize() override;
};
