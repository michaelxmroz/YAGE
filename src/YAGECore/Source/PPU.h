#pragma once
#include "Memory.h"
#include "PixelFIFO.h"
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

	~PPU();

	PPU(const PPU& other) = delete;
	PPU operator= (const PPU& other) = delete;

	void Init(Memory& memory);
	void Render(uint32_t mCycles, Memory& memory);
	void SwapBackbuffer();
	const void* GetFrameBuffer() const;

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

	void ScanOAM(const uint32_t& positionInLine, Memory& memory);
	void RenderNextPixel(Memory& memory);

	void TransitionToVBlank(Memory& memory);
	void TransitionToHBlank(Memory& memory);
	void TransitionToDraw(Memory& memory);
	void TransitionToOAMScan(Memory& memory);

	void DisableScreen(Memory& memory);
	void DrawPixels(Memory& memory, uint32_t& processedCycles);

	bool GetCurrentSprite(uint8_t& spriteIndex, uint8_t offset);

	static void CacheBackgroundPalette(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue, void* userData);
	static void LCDCWrite(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue, void* userData);


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
			, m_previousState(PPUState::VBlank)
			, m_spritePrefetchLine(0)
			, m_lineSpriteMask(0)
			, m_cycleDebt(0)
			, m_cachedBackgroundColors()
			, m_cyclesSinceStateChange(0)
			, m_fineScrollX(0)
		{}

		uint32_t m_totalCycles;
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

		uint32_t m_frameCount;

		PPUState m_state;
		PPUState m_previousState;
		uint32_t m_cyclesSinceStateChange;

		uint8_t m_fineScrollX;

		bool m_cachedBackgroundEnabled;
		RGBA m_cachedBackgroundColors[4];
	} data;
	void* m_activeFrame;
	void* m_backBuffer;


	// Inherited via ISerializable
	void Serialize(std::vector<Chunk>& chunks, std::vector<uint8_t>& data) override;
	void Deserialize(const Chunk* chunks, const uint32_t& chunkCount, const uint8_t* data, const uint32_t& dataSize) override;
};

