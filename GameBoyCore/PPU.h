#pragma once
#include "Memory.h"
#include "PixelFIFO.h"

#define MAX_SPRITES_PER_LINE 10

#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 144
#define SCREEN_SIZE SCREEN_WIDTH * SCREEN_HEIGHT

typedef void (*RenderFunc)(const void* image);

class PPU
{
public:
	PPU();

	~PPU();

	PPU(const PPU& other) = delete;
	PPU operator= (const PPU& other) = delete;

	void Init(Memory& memory, RenderFunc callback);

	void Render(uint32_t mCycles, Memory& memory);

	void DrawPixels(Memory& memory, uint32_t& processedCycles);

private:

	class PixelFetcher
	{
	public:
		void Reset();
		void Step(uint8_t x, uint8_t y, PixelFIFO& fifo, Memory& memory);
	private:
		enum class FetcherState
		{
			GetTile,
			ReadTileLow,
			ReadTileHigh,
			Sleep
		};

		FetcherState m_state;
		uint8_t m_x;
		uint8_t m_y;
		uint16_t m_tileAddr;
		uint8_t m_tileDataLow;
		uint8_t m_tileDataHigh;
	};

	void CheckForNewScanline(uint32_t totalCycles, Memory& memory);
	void ScanOAM(const uint32_t& positionInLine, Memory& memory, uint32_t& processedCycles);
	void RenderNextPixel(Memory& memory);

	void UpdateRenderListener();

	uint32_t m_totalCycles;
	uint8_t m_lineY;
	uint8_t m_lineX;
	uint8_t m_lineSpriteCount;
	SpriteAttributes m_lineSprites[MAX_SPRITES_PER_LINE];

	PixelFIFO m_spriteFIFO;
	PixelFIFO m_backgroundFIFO;

	PixelFetcher m_backgroundFetcher;

	void* m_renderedFrame;
	RenderFunc m_renderCallback;

	uint32_t m_frameCount;
};

