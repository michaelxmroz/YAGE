#pragma once
#include "Memory.h"
#include "PixelFIFO.h"

class PixelFetcher
{
public:
	PixelFetcher(bool spriteMode);

	void Reset();
	void FetchWindow();
	void SetSpriteAttributes(const SpriteAttributes* attributes);
	bool Step(uint8_t x, uint8_t y, PixelFIFO& fifo, uint32_t& processedCycles, Memory& memory);

private:
	enum class FetcherState
	{
		GetTile,
		ReadTileLow,
		ReadTileHigh,
		Sleep,
	};

	void PushPixels(PixelFIFO& fifo, uint8_t currentX);
	void GetBackgroundTile(Memory& memory, const uint8_t& y);

	const SpriteAttributes* m_spriteAttributes;
	FetcherState m_state;
	uint16_t m_tileAddr;
	uint8_t m_x;
	uint8_t m_y;
	uint8_t m_tileDataLow;
	uint8_t m_tileDataHigh;
	bool m_window;
	bool m_spriteMode;
};


