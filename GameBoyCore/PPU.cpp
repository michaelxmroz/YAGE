#include "PPU.h"
#include "Interrupts.h"
#include <algorithm>

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
#define VBLANK_LINE_Y 144

#define SPRITE_SINGLE_SIZE 8
#define SPRITE_DOUBLE_SIZE 16

PPU::PPU()
	: m_totalCycles(0)
	, m_lineY(0)
	, m_lineSpriteCount(0)
{
}

void PPU::Init(Memory& memory)
{
	memory.Write(LCDC_REGISTER, 0x91);
	memory.Write(STAT_REGISTER, 0x00);
	memory.Write(LYC_REGISTER, 0x00);
}

//TODO handle stat blocking for interrupts

void PPU::Render(uint32_t mCycles, Memory& memory)
{
	if (!IsControlFlagSet(ControlFlags::LCDEnable, memory))
	{
		//TODO reset the line counter & outputs?
		memory.Write(LY_REGISTER, 0);
		m_totalCycles = 0;
		m_lineY = 0xFF;
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
		}

		CheckForNewScanline(totalCycles, memory);

		//Do nothing during vblank
		if (m_lineY >= VBLANK_LINE_Y)
		{
			processedCycles = std::max<uint32_t>(targetCycles, 4);
		}
		else
		{
			uint32_t positionInLine = totalCycles % SCANLINE_DURATION;
			if (positionInLine < OAM_SCAN_DURATION)
			{
				uint16_t oamEntry = (positionInLine / 2);

				SpriteAttributes attr = memory.ReadOAMEntry(oamEntry);
				bool doubleSize = IsControlFlagSet(ControlFlags::ObjSize, memory);
				uint8_t doubleSizeAdjustment = doubleSize ? SPRITE_DOUBLE_SIZE : SPRITE_SINGLE_SIZE;

				if (m_lineSpriteCount < MAX_SPRITES_PER_LINE && m_lineY + SPRITE_DOUBLE_SIZE >= attr.m_posY && m_lineY + SPRITE_DOUBLE_SIZE < attr.m_posY + doubleSizeAdjustment)
				{
					m_lineSprites[m_lineSpriteCount] = attr;
					m_lineSpriteCount++;
				}
				processedCycles += 2;			
			}

			if (positionInLine == OAM_SCAN_DURATION)
			{
				SetModeFlag(3, memory);
				//TODO handle mode 3 FIFO
			}
			if (positionInLine >= OAM_SCAN_DURATION)
			{
				processedCycles += 2;
			}

			if (positionInLine == OAM_SCAN_DURATION + 168)
			{
				if (IsStatFlagSet(StatFlags::Mode0Interrupt, memory))
				{
					Interrupts::RequestInterrupt(Interrupts::Types::LCD_STAT, memory);
				}
				SetModeFlag(0, memory);
			}
			if (positionInLine >= OAM_SCAN_DURATION + 168)
			{
				processedCycles += 2;
			}
		}

		totalCycles = m_totalCycles + processedCycles;

		memory.Write(LY_REGISTER, m_lineY);
	} while (processedCycles < targetCycles);

	m_totalCycles += processedCycles;
}

void PPU::CheckForNewScanline(uint32_t totalCycles, Memory& memory)
{
	uint8_t newLineY = totalCycles / SCANLINE_DURATION;
	if (m_lineY != newLineY)
	{
		m_lineY = newLineY;
		if (m_lineY == VBLANK_LINE_Y)
		{
			memory.SetVRamAccess(Memory::VRamAccess::All);
			Interrupts::RequestInterrupt(Interrupts::Types::VBlank, memory);
			if (IsStatFlagSet(StatFlags::Mode1Interrupt, memory))
			{
				Interrupts::RequestInterrupt(Interrupts::Types::LCD_STAT, memory);
			}
			SetModeFlag(1, memory);
		}
		else if(m_lineY < VBLANK_LINE_Y)
		{
			memory.SetVRamAccess(Memory::VRamAccess::OAMBlocked);
			if (IsStatFlagSet(StatFlags::Mode2Interrupt, memory))
			{
				Interrupts::RequestInterrupt(Interrupts::Types::LCD_STAT, memory);
			}
			SetModeFlag(2, memory);
			m_lineSpriteCount = 0;
		}

		if (m_lineY == memory[LYC_REGISTER])
		{
			if (IsStatFlagSet(StatFlags::LCYEqLCInterrupt, memory))
			{
				Interrupts::RequestInterrupt(Interrupts::Types::LCD_STAT, memory);
			}
			SetStatFlag(StatFlags::LCYEqLC, memory);
		}
		else
		{
			ResetStatFlag(StatFlags::LCYEqLC, memory);
		}
	}
}

bool PPU::IsControlFlagSet(ControlFlags flag, Memory& memory) const
{
	return (memory[LCDC_REGISTER] & (1 << static_cast<uint8_t>(flag))) > 0;
}

bool PPU::IsStatFlagSet(StatFlags flag, Memory& memory) const
{
	return (memory[STAT_REGISTER] & (1 << static_cast<uint8_t>(flag))) > 0;
}

void PPU::SetStatFlag(StatFlags flag, Memory& memory) const
{
	memory.Write(STAT_REGISTER, memory[STAT_REGISTER] | (1 << static_cast<uint8_t>(flag)));
}

void PPU::ResetStatFlag(StatFlags flag, Memory& memory) const
{
	memory.Write(STAT_REGISTER, memory[STAT_REGISTER] & ~(1 << static_cast<uint8_t>(flag)));
}

void PPU::SetModeFlag(uint8_t mode, Memory& memory) const
{
	memory.Write(STAT_REGISTER, memory[STAT_REGISTER] | mode);
}

bool PPU::IsSpriteFlagSet(SpriteFlags flag, uint8_t flags) const
{
	return (flags & (1 << static_cast<uint8_t>(flag))) > 0;
}
