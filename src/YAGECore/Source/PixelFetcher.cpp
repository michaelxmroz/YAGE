#include "PixelFetcher.h"

#define LCDC_REGISTER 0xFF40 //LCD Control
#define SCY_REGISTER 0xFF42 // Scroll Y
#define SCX_REGISTER 0xFF43 // Scroll X
#define WY_REGISTER 0xFF4A // Window Y
#define WX_REGISTER 0xFF4B // Window X

#define TILE_MAP_DIMENSIONS 32

#define TILE_MAP_1 0x9800
#define TILE_MAP_2 0x9C00
#define TILE_DATA_BLOCK_1 0x8000
#define TILE_DATA_BLOCK_2 0x8800
#define TILE_DATA_BLOCK_3 0x9000
#define TILE_BYTE_SIZE 16
#define TILE_SIZE 8

enum class SpriteFlags
{
	PaletteNr = 4,
	XFlip = 5,
	YFlip = 6,
	BgOverObj = 7,
};

bool IsSpriteFlagSet(SpriteFlags flag, uint8_t flags)
{
	return (flags & (1 << static_cast<uint8_t>(flag))) > 0;
}

bool IsControlFlagSet(LCDControlFlags flag, Memory& memory)
{
	return (memory.ReadIO(LCDC_REGISTER) & (1 << static_cast<uint8_t>(flag))) > 0;
}

uint8_t GetColorForTilePixel(uint8_t index, uint8_t low, uint8_t high)
{
	index = 7 - index;
	return (((high >> index) & 0x1) << 1) | ((low >> index) & 0x1);
}

PixelFetcher::PixelFetcher(bool spriteMode) 
	: m_spriteMode(spriteMode)
	, m_spriteAttributes(nullptr)
	, m_state(FetcherState::GetTile)
	, m_tileAddr(0)
	, m_tileDataHigh(0)
	, m_tileDataLow(0)
	, m_window(false)
	, m_x(0)
	, m_y(0)
{
}

void PixelFetcher::Reset()
{
	m_state = FetcherState::GetTile;
	m_x = 0;
	m_y = 0;
	m_window = false;
	m_windowY = 0;
}

void PixelFetcher::FetchWindow(uint8_t windowY)
{
	m_window = true;
	m_windowY = windowY;
}

void PixelFetcher::SetSpriteAttributes(const SpriteAttributes* attributes)
{
	m_spriteAttributes = attributes;
}

bool PixelFetcher::Step(uint8_t x, uint8_t y, PixelFIFO& fifo, uint32_t& processedCycles, Memory& memory)
{
	switch (m_state)
	{
	case FetcherState::GetTile:
	{
		if (m_spriteMode)
		{
			uint16_t block = TILE_DATA_BLOCK_1;
			uint8_t yOffset = y + 16 - m_spriteAttributes->m_posY;
			bool doubleSize = IsControlFlagSet(LCDControlFlags::ObjSize, memory);
			if (IsSpriteFlagSet(SpriteFlags::YFlip, m_spriteAttributes->m_flags))
			{			
				uint8_t tileSize = TILE_SIZE;
				if (doubleSize)
				{
					tileSize *= 2;
				}
				yOffset = tileSize - 1 - yOffset;
			}

			uint16_t tileIndex = m_spriteAttributes->m_tileIndex;
			if (doubleSize)
			{
				tileIndex &= ~static_cast<uint16_t>(1);
			}

			m_tileAddr = tileIndex * TILE_BYTE_SIZE + yOffset * 2;
			m_tileAddr += block;

			processedCycles += 2;
		}
		else
		{
			GetBackgroundTile(memory, y);	
		}
		m_state = FetcherState::ReadTileLow;
	}
	break;
	case FetcherState::ReadTileLow:
	{
		m_tileDataLow = memory.ReadDirect(m_tileAddr);
		m_state = FetcherState::ReadTileHigh;
	}
	break;
	case FetcherState::ReadTileHigh:
	{
		m_tileDataHigh = memory.ReadDirect(m_tileAddr + 1);
		m_state = FetcherState::Sleep;
		if (fifo.Size() < 8 && !m_spriteMode)
		{
			PushPixels(fifo, x);
			return true;
		}
	}
	break;
	case FetcherState::Sleep:
	{
		if (fifo.Size() <= 8 || m_spriteMode)
		{
			PushPixels(fifo, x);
			return true;
		}
	}
	break;
	}

	return false;
}

void PixelFetcher::PushPixels(PixelFIFO& fifo, uint8_t currentX)
{
	if (m_spriteMode)
	{
		bool isOddPosition = m_spriteAttributes->m_posX != currentX + TILE_SIZE;

		uint8_t start = fifo.Size();
		uint8_t offscreenX = static_cast<uint8_t>(y::max(0, TILE_SIZE - static_cast<int16_t>(m_spriteAttributes->m_posX)));
		start = offscreenX;

		if (isOddPosition && start == 0)
		{
			Pixel pixel{
			0x00,
			false,
			false
			};
			fifo.Push(pixel);
		}

		for (uint8_t i = start; i < TILE_SIZE; ++i)
		{	
			bool flip = IsSpriteFlagSet(SpriteFlags::XFlip, m_spriteAttributes->m_flags);
			uint8_t index = flip ? TILE_SIZE - 1 - i : i;
			uint8_t color = GetColorForTilePixel(index, m_tileDataLow, m_tileDataHigh);
			Pixel pixel{
				color,
				IsSpriteFlagSet(SpriteFlags::PaletteNr, m_spriteAttributes->m_flags),
				IsSpriteFlagSet(SpriteFlags::BgOverObj, m_spriteAttributes->m_flags)
			};
			Pixel oldPixel;
			uint8_t fifoIndex = isOddPosition ? i + 1 : i;
			fifoIndex = y::max(0,fifoIndex - start);
			if (fifo.Size() > fifoIndex)
			{
				oldPixel = fifo.Get(fifoIndex);
				if (oldPixel.m_color == 0x00)
				{
					fifo.Replace(fifoIndex, pixel);
				}
			}
			else
			{
				fifo.Push(pixel);
			}
		}
	}
	else
	{
		for (uint8_t i = 0; i < TILE_SIZE; ++i)
		{
			uint8_t color = GetColorForTilePixel(i, m_tileDataLow, m_tileDataHigh);
			Pixel pixel{
				color,
				0,
				0
			};
			fifo.Push(pixel);
		}

		m_x++;
	}

	m_state = FetcherState::GetTile;
}

void PixelFetcher::GetBackgroundTile(Memory& memory, const uint8_t& y)
{
	uint16_t tileMapAddr = IsControlFlagSet(m_window ? LCDControlFlags::WindowTileMapArea : LCDControlFlags::BgTileMapArea, memory) ? TILE_MAP_2 : TILE_MAP_1;
	uint16_t tilePosX = m_x;
	uint16_t tilePosY = 0;
	uint8_t fineScrollY = 0;

	if (!m_window)
	{
		tilePosX = (memory.ReadIO(SCX_REGISTER) / TILE_SIZE + m_x) & 0x1F;
		uint8_t tileCoordY = (memory.ReadIO(SCY_REGISTER) + y) & 0xFF;
		tilePosY = tileCoordY / TILE_SIZE;
		fineScrollY = tileCoordY % TILE_SIZE;
	}
	else
	{
		uint8_t tileCoordY = m_windowY & 0xFF;
		tilePosY = tileCoordY / TILE_SIZE;
		fineScrollY = tileCoordY % TILE_SIZE;
	}

	uint16_t index = tileMapAddr + tilePosX + tilePosY * TILE_MAP_DIMENSIONS;
	uint8_t tileIndex = memory.ReadDirect(index);

	bool isUnsignedAddressing = IsControlFlagSet(LCDControlFlags::BgTileDataArea, memory);
	uint16_t dataAddr = isUnsignedAddressing ? TILE_DATA_BLOCK_1 : TILE_DATA_BLOCK_3;
	m_tileAddr = dataAddr;
	if (!isUnsignedAddressing && tileIndex > 127)
	{
		m_tileAddr = TILE_DATA_BLOCK_2;
		tileIndex -= 128;
	}
	m_tileAddr += tileIndex * TILE_BYTE_SIZE;
	m_tileAddr += fineScrollY * 2;
}
