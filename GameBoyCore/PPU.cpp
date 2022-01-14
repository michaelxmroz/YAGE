#include "PPU.h"
#include "Interrupts.h"
#include <algorithm>
#include "Logging.h"

#define LCDC_REGISTER 0xFF40 //LCD Control
#define STAT_REGISTER 0xFF41 // LCD Status
#define SCY_REGISTER 0xFF42 // Scroll Y
#define SCX_REGISTER 0xFF43 // Scroll X
#define LY_REGISTER 0xFF44 // LCD Y Coordinate
#define LYC_REGISTER 0xFF45 // LY Compare
#define WY_REGISTER 0xFF4A // Window Y
#define WX_REGISTER 0xFF4B // Window X
#define BGP_REGISTER 0xFF47 // Background Palette
#define OBJ0_REGISTER 0xFF48 // Sprite Palette 0
#define OBJ1_REGISTER 0xFF49 // Sprite Palette 1

#define TILE_MAP_1 0x9800
#define TILE_MAP_2 0x9C00
#define TILE_DATA_BLOCK_1 0x8000
#define TILE_DATA_BLOCK_2 0x8800
#define TILE_DATA_BLOCK_3 0x9000
#define TILE_BYTE_SIZE 16

#define TILE_MAP_DIMENSIONS 32

#define CYCLES_PER_FRAME 70224
#define MCYCLES_TO_CYCLES 4
#define OAM_SCAN_DURATION 80
#define SCANLINE_DURATION 456

#define SPRITE_SINGLE_SIZE 8
#define SPRITE_DOUBLE_SIZE 16

struct RGB
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

const RGB SCREEN_COLORS[4]
{
	{0xFF, 0xFF, 0xFF},
	{0xAA, 0xAA, 0xAA},
	{0x55, 0x55, 0x55},
	{0x00, 0x00, 0x00}
};

enum class ControlFlags
{
	BgEnable = 0,
	ObjEnable = 1,
	ObjSize = 2,
	BgTileMapArea = 3,
	BgTileDataArea = 4,
	WindowEnable = 5,
	WindowTileMapArea = 6,
	LCDEnable = 7
};

enum class StatFlags
{
	Mode = 0,
	LCYEqLC = 2,
	Mode0Interrupt = 3,
	Mode1Interrupt = 4,
	Mode2Interrupt = 5,
	LCYEqLCInterrupt = 6
};

enum class SpriteFlags
{
	PaletteNr = 4,
	XFlip = 5,
	YFlip = 6,
	BgOverObj = 7,
};

namespace PPUHelpers
{
	RGB ResolvePixelColor(uint8_t colorIndex, uint16_t paletteAddr, Memory& memory)
	{
		colorIndex = (memory[paletteAddr] >> (colorIndex * 2)) & 0x3;
		return SCREEN_COLORS[colorIndex];
	}

	bool IsControlFlagSet(ControlFlags flag, Memory& memory)
	{
		return (memory[LCDC_REGISTER] & (1 << static_cast<uint8_t>(flag))) > 0;
	}

	bool IsStatFlagSet(StatFlags flag, Memory& memory)
	{
		return (memory[STAT_REGISTER] & (1 << static_cast<uint8_t>(flag))) > 0;
	}

	void SetStatFlag(StatFlags flag, Memory& memory)
	{
		memory.Write(STAT_REGISTER, memory[STAT_REGISTER] | (1 << static_cast<uint8_t>(flag)));
	}

	void ResetStatFlag(StatFlags flag, Memory& memory)
	{
		memory.Write(STAT_REGISTER, memory[STAT_REGISTER] & ~(1 << static_cast<uint8_t>(flag)));
	}

	void SetModeFlag(uint8_t mode, Memory& memory)
	{
		memory.Write(STAT_REGISTER, memory[STAT_REGISTER] | mode);
	}

	bool IsSpriteFlagSet(SpriteFlags flag, uint8_t flags)
	{
		return (flags & (1 << static_cast<uint8_t>(flag))) > 0;
	}

	uint8_t GetColorForTilePixel(uint8_t index, uint8_t low, uint8_t high)
	{
		index = 7 - index;
		return (((high >> index) & 0x1) << 1) | ((low >> index) & 0x1);
	}
}

PPU::PPU()
	: m_totalCycles(0)
	, m_lineY(0)
	, m_lineSpriteCount(0)
	, m_frameCount(0)
	, m_renderCallback(nullptr)
	, m_backgroundFetcher()
	, m_lineSprites()
	, m_lineX(0)
{
	m_renderedFrame = new RGB[SCREEN_SIZE];
}

PPU::~PPU()
{
	delete m_renderedFrame;
}

void PPU::Init(Memory& memory, RenderFunc callback)
{
	memory.Write(LCDC_REGISTER, 0x91);
	memory.Write(STAT_REGISTER, 0x00);
	memory.Write(LYC_REGISTER, 0x00);
	memory.Write(SCY_REGISTER, 0x00);
	memory.Write(SCX_REGISTER, 0x00);
	memory.Write(BGP_REGISTER, 0xFC);
	memset(m_renderedFrame, 0, sizeof(RGB) * SCREEN_SIZE);

	memory.ClearVRAM();

	m_renderCallback = callback;
}

//TODO handle stat blocking for interrupts
//TODO refactor to state machine
void PPU::Render(uint32_t mCycles, Memory& memory)
{
	if (!PPUHelpers::IsControlFlagSet(ControlFlags::LCDEnable, memory))
	{
		if (m_lineY != 0xFF)
		{
			memory.Write(LY_REGISTER, 0);
			m_totalCycles = 0;
			m_lineY = 0xFF;
			PPUHelpers::SetModeFlag(0, memory);
			memset(m_renderedFrame, 0, SCREEN_SIZE);
			UpdateRenderListener();
		}

		return;
	}

	uint32_t targetCycles = mCycles * MCYCLES_TO_CYCLES;
	uint32_t processedCycles = 0;
	uint32_t totalCycles = m_totalCycles;
	do
	{
		if (totalCycles >= CYCLES_PER_FRAME)
		{
			totalCycles -= CYCLES_PER_FRAME;
			m_totalCycles = totalCycles;
			m_frameCount++;
		}

		CheckForNewScanline(totalCycles, memory);

		//Do nothing during vblank
		if (m_lineY >= SCREEN_HEIGHT)
		{
			processedCycles = std::max<uint32_t>(targetCycles, 4);
		}
		else
		{
			uint32_t positionInLine = totalCycles % SCANLINE_DURATION;

			//OAM Scan
			if (positionInLine < OAM_SCAN_DURATION)
			{
				ScanOAM(positionInLine, memory, processedCycles);
			}

			//Pixel Drawing
			if (positionInLine == OAM_SCAN_DURATION)
			{
				PPUHelpers::SetModeFlag(3, memory);
				m_lineX = 0;
				m_backgroundFIFO.Clear();
				m_spriteFIFO.Clear();
				m_backgroundFetcher.Reset();
			}

			if (m_lineX < SCREEN_WIDTH)
			{
				DrawPixels(memory, processedCycles);
			}

			//HBlank
			if (m_lineX == SCREEN_WIDTH)
			{
				if (PPUHelpers::IsStatFlagSet(StatFlags::Mode0Interrupt, memory))
				{
					Interrupts::RequestInterrupt(Interrupts::Types::LCD_STAT, memory);
				}
				PPUHelpers::SetModeFlag(0, memory);
				m_lineX++;
			}

			if (m_lineX > SCREEN_WIDTH)
			{
				processedCycles += 2;
			}
		}

		totalCycles = m_totalCycles + processedCycles;

		memory.Write(LY_REGISTER, m_lineY);
	} while (processedCycles < targetCycles);

	m_totalCycles += processedCycles;
}

void PPU::DrawPixels(Memory& memory, uint32_t& processedCycles)
{
	if (PPUHelpers::IsControlFlagSet(ControlFlags::BgEnable, memory))
	{
		m_backgroundFetcher.Step(m_lineX, m_lineY, m_backgroundFIFO, memory);
	}

	if (m_lineX == 0)
	{
		uint8_t fineScroll = memory[SCX_REGISTER] & 0x7;
		if (fineScroll > 0 && m_backgroundFIFO.Size() > 8)
		{
			for (uint8_t i = 0; i < fineScroll; ++i)
			{
				m_backgroundFIFO.Pop();
			}
		}
	}

	//TODO sprites
	if (m_backgroundFIFO.Size() > 8)
	{
		RenderNextPixel(memory);
		RenderNextPixel(memory);
	}

	processedCycles += 2;
}

void PPU::RenderNextPixel(Memory& memory)
{
	Pixel pixel = m_backgroundFIFO.Pop();
	RGB pixelColor = PPUHelpers::ResolvePixelColor(pixel.m_color, BGP_REGISTER, memory);
	uint32_t renderIndex = m_lineX + m_lineY * SCREEN_WIDTH;
	reinterpret_cast<RGB*>(m_renderedFrame)[renderIndex] = pixelColor;
	m_lineX++;
}

void PPU::ScanOAM(const uint32_t& positionInLine, Memory& memory, uint32_t& processedCycles)
{
	uint8_t oamEntry = (positionInLine / 2);

	SpriteAttributes attr = memory.ReadOAMEntry(oamEntry);
	bool doubleSize = PPUHelpers::IsControlFlagSet(ControlFlags::ObjSize, memory);
	uint8_t doubleSizeAdjustment = doubleSize ? SPRITE_DOUBLE_SIZE : SPRITE_SINGLE_SIZE;

	if (m_lineSpriteCount < MAX_SPRITES_PER_LINE && m_lineY + SPRITE_DOUBLE_SIZE >= attr.m_posY && m_lineY + SPRITE_DOUBLE_SIZE < attr.m_posY + doubleSizeAdjustment)
	{
		m_lineSprites[m_lineSpriteCount] = attr;
		m_lineSpriteCount++;
	}
	processedCycles += 2;
}

void PPU::UpdateRenderListener()
{
	if (m_renderCallback != nullptr)
	{
		m_renderCallback(m_renderedFrame);
	}
}

void PPU::CheckForNewScanline(uint32_t totalCycles, Memory& memory)
{
	uint8_t newLineY = totalCycles / SCANLINE_DURATION;
	if (m_lineY != newLineY)
	{
		m_lineY = newLineY;
		if (m_lineY == SCREEN_HEIGHT)
		{
			memory.SetVRamAccess(Memory::VRamAccess::All);
			Interrupts::RequestInterrupt(Interrupts::Types::VBlank, memory);
			if (PPUHelpers::IsStatFlagSet(StatFlags::Mode1Interrupt, memory))
			{
				Interrupts::RequestInterrupt(Interrupts::Types::LCD_STAT, memory);
			}
			PPUHelpers::SetModeFlag(1, memory);
			if(m_frameCount == 1000)
				UpdateRenderListener();
		}
		else if(m_lineY < SCREEN_HEIGHT)
		{
			memory.SetVRamAccess(Memory::VRamAccess::OAMBlocked);
			if (PPUHelpers::IsStatFlagSet(StatFlags::Mode2Interrupt, memory))
			{
				Interrupts::RequestInterrupt(Interrupts::Types::LCD_STAT, memory);
			}
			PPUHelpers::SetModeFlag(2, memory);
			m_lineSpriteCount = 0;
		}

		if (m_lineY == memory[LYC_REGISTER])
		{
			if (PPUHelpers::IsStatFlagSet(StatFlags::LCYEqLCInterrupt, memory))
			{
				Interrupts::RequestInterrupt(Interrupts::Types::LCD_STAT, memory);
			}
			PPUHelpers::SetStatFlag(StatFlags::LCYEqLC, memory);
		}
		else
		{
			PPUHelpers::ResetStatFlag(StatFlags::LCYEqLC, memory);
		}
	}
}

void PPU::PixelFetcher::Reset()
{
	m_state = FetcherState::GetTile;
	m_x = 0;
	m_y = 0;
}

void PPU::PixelFetcher::Step(uint8_t x, uint8_t y, PixelFIFO& fifo, Memory& memory)
{
	switch (m_state)
	{
	case FetcherState::GetTile:
	{
		uint16_t tileMapAddr = PPUHelpers::IsControlFlagSet(ControlFlags::BgTileMapArea, memory) ? TILE_MAP_2 : TILE_MAP_1;
		uint16_t tilePosX = (memory[SCX_REGISTER] / 8 + m_x) & 0x1F;
		m_y = (memory[SCY_REGISTER] + y) & 0xFF;
		uint16_t tilePosY = m_y / 8;
		uint8_t fineScrollY = m_y % 8;
		uint16_t index = tileMapAddr + tilePosX + tilePosY * TILE_MAP_DIMENSIONS;
		uint8_t tileIndex = memory[index];

		bool isUnsignedAddressing = PPUHelpers::IsControlFlagSet(ControlFlags::BgTileDataArea, memory);
		uint16_t dataAddr = isUnsignedAddressing ? TILE_DATA_BLOCK_1 : TILE_DATA_BLOCK_3;
		m_tileAddr = dataAddr;
		if (!isUnsignedAddressing && tileIndex > 127)
		{
			m_tileAddr = TILE_DATA_BLOCK_2;
			tileIndex -= 127;
		}
		m_tileAddr += tileIndex * TILE_BYTE_SIZE;
		m_tileAddr += fineScrollY * 2;

		m_state = FetcherState::ReadTileLow;
	}
		break;
	case FetcherState::ReadTileLow:
	{
		m_tileDataLow = memory[m_tileAddr];
		m_state = FetcherState::ReadTileHigh;
	}
		break;
	case FetcherState::ReadTileHigh:
	{
		m_tileDataHigh = memory[m_tileAddr + 1];
		m_state = FetcherState::Sleep;
	}
		break;
	case FetcherState::Sleep:
	{
		if (fifo.Size() <= 8)
		{
			for (uint8_t i = 0; i < 8; ++i)
			{
				uint8_t color = PPUHelpers::GetColorForTilePixel(i, m_tileDataLow, m_tileDataHigh);
				Pixel pixel{
					color,
					0,
					0
				};
				fifo.Push(pixel);
			}
			m_state = FetcherState::GetTile;
			m_x++;
		}
	}
		break;
	}

}
