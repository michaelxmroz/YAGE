#include "PPU.h"
#include "VirtualMachine.h"
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

#define CYCLES_PER_FRAME 70224
#define MCYCLES_TO_CYCLES 4
#define OAM_SCAN_DURATION 80
#define SCANLINE_DURATION 456
#define MAX_LINES_Y 154

#define SPRITE_SINGLE_SIZE 8
#define SPRITE_DOUBLE_SIZE 16

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

const RGBA SCREEN_COLORS[4]
{
	{0xFF, 0xFF, 0xFF, 0xFF},
	{0xAA, 0xAA, 0xAA, 0xFF},
	{0x55, 0x55, 0x55, 0xFF},
	{0x00, 0x00, 0x00, 0xFF}
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

namespace PPUHelpers
{
	RGBA ResolvePixelColor(uint8_t colorIndex, uint16_t paletteAddr, Memory& memory)
	{
		colorIndex = (memory[paletteAddr] >> (colorIndex * 2)) & 0x3;
		return SCREEN_COLORS[colorIndex];
	}

	bool IsControlFlagSet(LCDControlFlags flag, Memory& memory)
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
		memory.Write(STAT_REGISTER, (memory[STAT_REGISTER] & 0xFC) | mode);
	}

	bool IsNewScanline(uint32_t totalCycles, uint8_t& currentLine, Memory& memory)
	{
		uint8_t newLineY = totalCycles / SCANLINE_DURATION;
		if (currentLine != newLineY)
		{
			currentLine = newLineY % MAX_LINES_Y;
			if (currentLine == memory[LYC_REGISTER])
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

			return true;
		}
		return false;
	}
}

PPU::PPU()
	: m_totalCycles(0)
	, m_lineY(0)
	, m_lineSpriteCount(0)
	, m_frameCount(0)
	, m_renderCallback(nullptr)
	, m_backgroundFetcher(false)
	, m_spriteFetcher(true)
	, m_lineSprites()
	, m_lineX(0)
	, m_windowState(WindowState::NoWindow)
	, m_state(PPUState::OAMScan)
	, m_spritePrefetchLine(0)
	, m_lineSpriteMask(0)
{
	m_renderedFrame = new RGBA[EmulatorConstants::SCREEN_SIZE];
}

PPU::~PPU()
{
	delete m_renderedFrame;
}

void PPU::Init(Memory& memory)
{
	memory.Write(LCDC_REGISTER, 0x91);
	memory.Write(STAT_REGISTER, 0x00);
	memory.Write(LYC_REGISTER, 0x00);
	memory.Write(SCY_REGISTER, 0x00);
	memory.Write(SCX_REGISTER, 0x00);
	memory.Write(BGP_REGISTER, 0xFC);
	memory.Write(OBJ0_REGISTER, 0x00);
	memory.Write(OBJ1_REGISTER, 0x00);
	memset(m_renderedFrame, 0, sizeof(RGBA) * EmulatorConstants::SCREEN_SIZE);

	memory.ClearVRAM();

	TransitionToOAMScan(memory);
}

//TODO handle stat blocking for interrupts
bool PPU::Render(uint32_t mCycles, Memory& memory)
{
	if (!PPUHelpers::IsControlFlagSet(LCDControlFlags::LCDEnable, memory))
	{
		if (m_totalCycles != 0)
		{
			DisableScreen(memory);
		}
		return false;
	}

	uint32_t targetCycles = mCycles * MCYCLES_TO_CYCLES;
	uint32_t processedCycles = 0;
	uint32_t totalCycles = m_totalCycles;
	do
	{
		switch (m_state)
		{
			case PPUState::OAMScan:
			{
				uint32_t positionInLine = totalCycles % SCANLINE_DURATION;
				ScanOAM(positionInLine, memory, processedCycles);
				if (positionInLine == OAM_SCAN_DURATION)
				{
					TransitionToDraw(memory);
				}
			}
			break;
			case PPUState::Drawing:
			{
				DrawPixels(memory, processedCycles);

				if (m_lineX == EmulatorConstants::SCREEN_WIDTH)
				{
					TransitionToHBlank(memory);
				}
			}
			break;
			case PPUState::HBlank:
			{
				if (PPUHelpers::IsNewScanline(totalCycles, m_lineY, memory))
				{
					if (m_totalCycles % 2 != 0)
					{
						m_totalCycles--;
					}

					if (m_lineY == EmulatorConstants::SCREEN_HEIGHT)
					{
						TransitionToVBlank(memory);
					}
					else
					{
						TransitionToOAMScan(memory);
					}
				}
				else
				{
					processedCycles += 2;
				}
			}
			break;
			case PPUState::VBlank:
			{
				if (PPUHelpers::IsNewScanline(totalCycles, m_lineY, memory))
				{
					if (m_lineY == 0)
					{
						totalCycles = 0;
						m_totalCycles = totalCycles;
						m_frameCount++;
						TransitionToOAMScan(memory);
						return true;
					}
				}
				else
				{
					processedCycles += 2;
				}
			}
			break;
		}

		totalCycles = m_totalCycles + processedCycles;
	} while (processedCycles < targetCycles);

	memory.Write(LY_REGISTER, m_lineY);
	m_totalCycles += processedCycles;
	return false;
}

const void* PPU::GetFrameBuffer() const
{
	return m_renderedFrame;
}

void PPU::TransitionToVBlank(Memory& memory)
{
	if (m_frameCount == 1000)
		UpdateRenderListener();

	Interrupts::RequestInterrupt(Interrupts::Types::VBlank, memory);
	if (PPUHelpers::IsStatFlagSet(StatFlags::Mode1Interrupt, memory))
	{
		Interrupts::RequestInterrupt(Interrupts::Types::LCD_STAT, memory);
	}
	PPUHelpers::SetModeFlag(static_cast<uint8_t>(PPUState::VBlank), memory);
	m_state = PPUState::VBlank;
}

void PPU::TransitionToHBlank(Memory& memory)
{
	if (PPUHelpers::IsStatFlagSet(StatFlags::Mode0Interrupt, memory))
	{
		Interrupts::RequestInterrupt(Interrupts::Types::LCD_STAT, memory);
	}
	memory.SetVRamAccess(Memory::VRamAccess::All);
	PPUHelpers::SetModeFlag(static_cast<uint8_t>(PPUState::HBlank), memory);
	m_state = PPUState::HBlank;
}

void PPU::TransitionToDraw(Memory& memory)
{
	m_lineX = 0;
	m_backgroundFIFO.Clear();
	m_spriteFIFO.Clear();
	m_backgroundFetcher.Reset();
	m_spriteFetcher.Reset();

	memory.SetVRamAccess(Memory::VRamAccess::VRamOAMBlocked);
	PPUHelpers::SetModeFlag(static_cast<uint8_t>(PPUState::Drawing), memory);
	m_state = PPUState::Drawing;
}

void PPU::TransitionToOAMScan(Memory& memory)
{
	m_lineSpriteCount = 0;
	m_lineSpriteMask = 0;
	m_spritePrefetchLine = 0;
	m_windowState = PPUHelpers::IsControlFlagSet(LCDControlFlags::WindowEnable, memory) && m_lineY >= memory[WY_REGISTER] ? WindowState::InScanline : WindowState::NoWindow;

	if (PPUHelpers::IsStatFlagSet(StatFlags::Mode2Interrupt, memory))
	{
		Interrupts::RequestInterrupt(Interrupts::Types::LCD_STAT, memory);
	}
	PPUHelpers::SetModeFlag(static_cast<uint8_t>(PPUState::OAMScan), memory);
	memory.SetVRamAccess(Memory::VRamAccess::OAMBlocked);
	m_state = PPUState::OAMScan;
}

void PPU::DisableScreen(Memory& memory)
{
	memory.Write(LY_REGISTER, 0);
	m_totalCycles = 0;
	m_lineY = 0x0;
	PPUHelpers::SetModeFlag(static_cast<uint8_t>(PPUState::HBlank), memory);
	memset(m_renderedFrame, 1, sizeof(RGBA) * EmulatorConstants::SCREEN_SIZE);
	TransitionToOAMScan(memory);
	UpdateRenderListener();
}

void PPU::DrawPixels(Memory& memory, uint32_t& processedCycles)
{
	processedCycles += 2;

	if (m_windowState == WindowState::InScanline && m_lineX == memory[WX_REGISTER] - 7)
	{
		m_backgroundFetcher.Reset();
		m_backgroundFIFO.Clear();
		m_backgroundFetcher.FetchWindow();
		m_windowState = WindowState::Draw;
	}
	
	uint8_t currentSpriteIndex;

	if (m_lineX == 0)
	{
		while (m_spritePrefetchLine < SPRITE_SINGLE_SIZE)
		{
			if (GetCurrentSprite(currentSpriteIndex, SPRITE_SINGLE_SIZE - m_spritePrefetchLine))
			{
				m_spriteFetcher.SetSpriteAttributes(&m_lineSprites[currentSpriteIndex]);
				bool fetchFinished = m_spriteFetcher.Step(m_lineX, m_lineY, m_spriteFIFO, processedCycles, memory);
				if (fetchFinished)
				{
					m_lineSpriteMask |= (1 << currentSpriteIndex);
				}
				return;
			}
			m_spritePrefetchLine++;
		}
	}

	if (GetCurrentSprite(currentSpriteIndex, 8))
	{
		m_spriteFetcher.SetSpriteAttributes(&m_lineSprites[currentSpriteIndex]);
		bool fetchFinished = m_spriteFetcher.Step(m_lineX, m_lineY, m_spriteFIFO, processedCycles, memory);
		if (fetchFinished)
		{
			m_lineSpriteMask |= (1 << currentSpriteIndex);
		}
		return;
	}
	else
	{
		m_backgroundFetcher.Step(m_lineX, m_lineY, m_backgroundFIFO, processedCycles, memory);
	}
	
	if (m_lineX == 0)
	{
		uint8_t fineScroll = memory[SCX_REGISTER] & 0x7;
		if (fineScroll > 0 && m_backgroundFIFO.Size() > SPRITE_SINGLE_SIZE)
		{
			for (uint8_t i = 0; i < fineScroll; ++i)
			{
				m_backgroundFIFO.Pop();
			}

			processedCycles += fineScroll;
		}
	}

	//TODO sprites
	if (m_backgroundFIFO.Size() > SPRITE_SINGLE_SIZE )
	{
		RenderNextPixel(memory);
		RenderNextPixel(memory);
	}
}

void PPU::RenderNextPixel(Memory& memory)
{
	Pixel bgPixel = m_backgroundFIFO.Pop();
	RGBA pixelColor = PPUHelpers::ResolvePixelColor(bgPixel.m_color, BGP_REGISTER, memory);
	if (!PPUHelpers::IsControlFlagSet(LCDControlFlags::BgEnable, memory))
	{
		pixelColor = SCREEN_COLORS[0];
	}

	if (m_spriteFIFO.Size() > 0)
	{
		Pixel spritePixel = m_spriteFIFO.Pop();
		RGBA spritePixelColor = PPUHelpers::ResolvePixelColor(spritePixel.m_color, spritePixel.m_palette == 0 ? OBJ0_REGISTER : OBJ1_REGISTER, memory);

		if (PPUHelpers::IsControlFlagSet(LCDControlFlags::ObjEnable, memory) && 
			(spritePixel.m_color != 0 && (!spritePixel.m_backgroundPriority || pixelColor == SCREEN_COLORS[0])))
		{
			pixelColor = spritePixelColor;
		}
	}

	uint32_t renderIndex = m_lineX + m_lineY * EmulatorConstants::SCREEN_WIDTH;
	reinterpret_cast<RGBA*>(m_renderedFrame)[renderIndex] = pixelColor;
	m_lineX++;
}

void PPU::ScanOAM(const uint32_t& positionInLine, Memory& memory, uint32_t& processedCycles)
{
	uint8_t oamEntry = (positionInLine / 2);

	SpriteAttributes attr = memory.ReadOAMEntry(oamEntry);
	bool doubleSize = PPUHelpers::IsControlFlagSet(LCDControlFlags::ObjSize, memory);
	uint8_t doubleSizeAdjustment = doubleSize ? 0 : SPRITE_SINGLE_SIZE;

	bool isInLine = m_lineY + doubleSizeAdjustment < attr.m_posY && m_lineY + SPRITE_DOUBLE_SIZE >= attr.m_posY;

	if (m_lineSpriteCount < MAX_SPRITES_PER_LINE && isInLine)
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

bool PPU::GetCurrentSprite(uint8_t& spriteIndex, uint8_t offset)
{
	for (uint8_t i = 0; i < m_lineSpriteCount; ++i)
	{
		int16_t posDifference = m_lineSprites[i].m_posX - (m_lineX + offset);
		bool isAvailable = (m_lineSpriteMask & (1 << i)) == 0;
		if (isAvailable && (posDifference == 0 || posDifference == 1))
		{
			spriteIndex = i;
			return true;
		}
	}
	return false;
}
