#pragma once
#include "Memory.h"
#include "PixelFIFO.h"
#include "PixelFetcher.h"

#define MAX_SPRITES_PER_LINE 10

typedef void (*RenderFunc)(const void* image);

class PPU
{
public:
	PPU();

	~PPU();

	PPU(const PPU& other) = delete;
	PPU operator= (const PPU& other) = delete;

	void Init(Memory& memory);
	bool Render(uint32_t mCycles, Memory& memory);
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

	void ScanOAM(const uint32_t& positionInLine, Memory& memory, uint32_t& processedCycles);
	void RenderNextPixel(Memory& memory);

	void TransitionToVBlank(Memory& memory);
	void TransitionToHBlank(Memory& memory);
	void TransitionToDraw(Memory& memory);
	void TransitionToOAMScan(Memory& memory);

	void DisableScreen(Memory& memory);
	void DrawPixels(Memory& memory, uint32_t& processedCycles);
	void UpdateRenderListener();

	bool GetCurrentSprite(uint8_t& spriteIndex, uint8_t offset);

	uint32_t m_totalCycles;
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

	void* m_renderedFrame;
	RenderFunc m_renderCallback;

	uint32_t m_frameCount;

	PPUState m_state;
};

