#include "PPU.h"
#include "VirtualMachine.h"
#include "Interrupts.h"
#include "Logging.h"
#include "Allocator.h"

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

#define DOTS_PER_FRAME 70224
#define OAM_SCAN_DURATION 80
#define SCANLINE_DURATION 456
#define MAX_LINES_Y 154

#define SPRITE_SINGLE_SIZE 8
#define SPRITE_DOUBLE_SIZE 16

const RGBA SCREEN_COLORS[4]
{
	{0xE0, 0xF8, 0xD8, 0xFF},
	{0x88, 0xC0, 0x70, 0xFF},
	{0x34, 0x68, 0x56, 0xFF},
	{0x08, 0x18, 0x20, 0xFF}
};

enum class StatFlags : uint32_t
{
	Mode = 0,
	LCYEqLC = 2,
	Mode0Interrupt = 3,
	Mode1Interrupt = 4,
	Mode2Interrupt = 5,
	LCYEqLCInterrupt = 6
};

const StatFlags ModeIndexToStatFlags[3]
{
	StatFlags::Mode0Interrupt,
	StatFlags::Mode1Interrupt,
	StatFlags::Mode2Interrupt
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
		return (memory.ReadIO(LCDC_REGISTER) & (1 << static_cast<uint8_t>(flag))) > 0;
	}

	bool IsStatFlagSet(StatFlags flag, Memory& memory)
	{
		return (memory.ReadIO(STAT_REGISTER) & (1 << static_cast<uint8_t>(flag))) > 0;
	}

	void SetStatFlag(StatFlags flag, Memory& memory)
	{
		memory.WriteIO(STAT_REGISTER, memory.ReadIO(STAT_REGISTER) | (1 << static_cast<uint8_t>(flag)));
	}

	void ResetStatFlag(StatFlags flag, Memory& memory)
	{
		memory.WriteIO(STAT_REGISTER, memory.ReadIO(STAT_REGISTER) & ~(1 << static_cast<uint8_t>(flag)));
	}

	void SetModeFlag(uint8_t mode, Memory& memory)
	{
		memory.WriteIO(STAT_REGISTER, (memory.ReadIO(STAT_REGISTER) & 0xFC) | mode);
	}

	bool IsNewScanline(uint32_t totalCycles, uint8_t& currentLine, Memory& memory)
	{
		uint8_t newLineY = totalCycles / SCANLINE_DURATION;
		if (currentLine != newLineY)
		{
			currentLine = newLineY % MAX_LINES_Y;
			return true;
		}
		return false;
	}
}

PPU::PPU(GamestateSerializer* serializer) : ISerializable(serializer, ChunkId::PPU)
	, data()
{
	m_activeFrame = Y_NEW_A(RGBA, EmulatorConstants::SCREEN_SIZE);
	m_backBuffer = Y_NEW_A(RGBA, EmulatorConstants::SCREEN_SIZE);
}

PPU::~PPU()
{
	Y_DELETE_A(m_activeFrame);
	Y_DELETE_A(m_backBuffer);
}

void PPU::Init(Memory& memory)
{
	memory.RegisterCallback(BGP_REGISTER, CacheBackgroundPalette, this);
	memory.RegisterCallback(LCDC_REGISTER, LCDCWrite, this);

	memory.WriteIO(LCDC_REGISTER, 0x91);
	memory.WriteIO(STAT_REGISTER, 0x00);
	memory.WriteIO(LYC_REGISTER, 0x00);
	memory.WriteIO(SCY_REGISTER, 0x00);
	memory.WriteIO(SCX_REGISTER, 0x00);
	memory.WriteIO(BGP_REGISTER, 0xFC);
	memory.WriteIO(OBJ0_REGISTER, 0x00);
	memory.WriteIO(OBJ1_REGISTER, 0x00);
	memset_y(m_activeFrame, 0, sizeof(RGBA) * EmulatorConstants::SCREEN_SIZE);
	memset_y(m_backBuffer, 0, sizeof(RGBA) * EmulatorConstants::SCREEN_SIZE);


	memory.AddIOUnusedBitsOverride(STAT_REGISTER, 0b10000000);
	memory.AddIOReadOnlyRange(STAT_REGISTER, 0b00000111);

	memory.ClearVRAM();

	TransitionToOAMScan(memory);
}

//TODO handle stat blocking for interrupts
void PPU::Render(uint32_t mCycles, Memory& memory)
{
	if (!PPUHelpers::IsControlFlagSet(LCDControlFlags::LCDEnable, memory))
	{
		return;
	}

	int32_t targetCycles = mCycles * MCYCLES_TO_CYCLES;
	targetCycles += data.m_cycleDebt;
	uint32_t processedCycles = 0;
	uint32_t totalCycles = data.m_totalCycles;

	if (data.m_previousState != data.m_state)
	{
		data.m_previousState = data.m_state;
		data.m_cyclesSinceStateChange = 0;
		PPUHelpers::ResetStatFlag(StatFlags::LCYEqLC, memory);
	}


	while (static_cast<int32_t>(processedCycles) < targetCycles)
	{
		switch (data.m_state)
		{
			case PPUState::OAMScan:
			{
				uint32_t positionInLine = totalCycles % SCANLINE_DURATION;
				ScanOAM(positionInLine, memory);
				processedCycles += 2;
				positionInLine += 2;
				if (positionInLine == OAM_SCAN_DURATION)
				{
					TransitionToDraw(memory);
				}
			}
			break;
			case PPUState::Drawing:
			{
				DrawPixels(memory, processedCycles);

				if (data.m_lineX == EmulatorConstants::SCREEN_WIDTH)
				{
					TransitionToHBlank(memory);
				}
			}
			break;
			case PPUState::HBlank:
			{
				processedCycles += 2;

				if (PPUHelpers::IsNewScanline(data.m_totalCycles + processedCycles, data.m_lineY, memory))
				{
					if (data.m_totalCycles % 2 != 0)
					{
						data.m_totalCycles--;
					}

					if (data.m_lineY == EmulatorConstants::SCREEN_HEIGHT)
					{
						SwapBackbuffer();

						TransitionToVBlank(memory);
					}
					else
					{
						TransitionToOAMScan(memory);
					}
				}
				else
				{
					
				}
			}
			break;
			case PPUState::VBlank:
			{
				if (PPUHelpers::IsNewScanline(totalCycles, data.m_lineY, memory))
				{
					if (data.m_lineY == 0)
					{
						data.m_cycleDebt = 0;

						totalCycles = 0;
						data.m_totalCycles = totalCycles;
						data.m_frameCount++;
						TransitionToOAMScan(memory);
					}
				}
				else
				{
					processedCycles += 2;
				}
			}
			break;
		}

		totalCycles = data.m_totalCycles + processedCycles;
	}

	data.m_cyclesSinceStateChange += processedCycles;

	uint32_t writtenLine = data.m_lineY;

	if (data.m_lineY == MAX_LINES_Y - 1 && data.m_cyclesSinceStateChange > 0)
	{
		writtenLine = 0;
	}

	memory.WriteIO(LY_REGISTER, writtenLine);
	if (writtenLine == memory.ReadIO(LYC_REGISTER))
	{
		PPUHelpers::SetStatFlag(StatFlags::LCYEqLC, memory);
	}
	else
	{
		PPUHelpers::ResetStatFlag(StatFlags::LCYEqLC, memory);
	}


	if (data.m_cyclesSinceStateChange == 4)
	{
		if ((data.m_state == PPUState::OAMScan || data.m_state == PPUState::VBlank) && writtenLine == memory.ReadIO(LYC_REGISTER))
		{
			if (PPUHelpers::IsStatFlagSet(StatFlags::LCYEqLCInterrupt, memory))
			{
				Interrupts::RequestInterrupt(Interrupts::Types::LCD_STAT, memory);
			}
			PPUHelpers::SetStatFlag(StatFlags::LCYEqLC, memory);
		}

		if (data.m_state == PPUState::VBlank)
		{
			Interrupts::RequestInterrupt(Interrupts::Types::VBlank, memory);
		}

		if (PPUHelpers::IsStatFlagSet(ModeIndexToStatFlags[static_cast<uint32_t>(data.m_state)], memory))
		{
			Interrupts::RequestInterrupt(Interrupts::Types::LCD_STAT, memory);
		}

		PPUHelpers::SetModeFlag(static_cast<uint8_t>(data.m_state), memory);
	}

	data.m_cycleDebt =  targetCycles - processedCycles;

	data.m_totalCycles += processedCycles;
}

void PPU::SwapBackbuffer()
{
	RGBA* swap = m_backBuffer;
	m_backBuffer = m_activeFrame;
	m_activeFrame = swap;
}

const void* PPU::GetFrameBuffer() const
{
	return m_backBuffer;
}

void PPU::TransitionToVBlank(Memory& memory)
{
	data.m_state = PPUState::VBlank;
}

void PPU::TransitionToHBlank(Memory& memory)
{
	memory.SetVRamAccess(Memory::VRamAccess::All);
	data.m_state = PPUState::HBlank;
}

void PPU::TransitionToDraw(Memory& memory)
{
	data.m_lineX = 0;
	data.m_backgroundFIFO.Clear();
	data.m_spriteFIFO.Clear();
	data.m_backgroundFetcher.Reset();
	data.m_spriteFetcher.Reset();

	memory.SetVRamAccess(Memory::VRamAccess::VRamOAMBlocked);
	data.m_state = PPUState::Drawing;
}

void PPU::TransitionToOAMScan(Memory& memory)
{
	data.m_fineScrollX = memory.ReadIO(SCX_REGISTER) & 0x7;
	data.m_lineSpriteCount = 0;
	data.m_lineSpriteMask = 0;
	data.m_spritePrefetchLine = 0;
	data.m_windowState = PPUHelpers::IsControlFlagSet(LCDControlFlags::WindowEnable, memory) && data.m_lineY >= memory.ReadIO(WY_REGISTER) ? WindowState::InScanline : WindowState::NoWindow;
	memory.SetVRamAccess(Memory::VRamAccess::OAMBlocked);
	data.m_state = PPUState::OAMScan;
}

void PPU::DisableScreen(Memory& memory)
{

	memory.WriteIO(LY_REGISTER, 0);
	data.m_totalCycles = 4; // PPU starts a bit delayed when turned on
	data.m_cycleDebt = 0;
	data.m_lineY = 0x0;
	PPUHelpers::SetModeFlag(static_cast<uint8_t>(PPUState::HBlank), memory);
	memset_y(m_activeFrame, 1, sizeof(RGBA) * EmulatorConstants::SCREEN_SIZE);
	memory.SetVRamAccess(Memory::VRamAccess::All);
}

void PPU::DrawPixels(Memory& memory, uint32_t& processedCycles)
{
	processedCycles += 2;

	if (data.m_windowState == WindowState::InScanline && data.m_lineX + 7 >= memory.ReadIO(WX_REGISTER))
	{
		data.m_backgroundFetcher.Reset();
		data.m_backgroundFIFO.Clear();
		data.m_backgroundFetcher.FetchWindow();
		data.m_windowState = WindowState::Draw;
		data.m_fineScrollX = 0x7 - memory.ReadIO(WX_REGISTER) & 0x7;
	}
	
	uint8_t currentSpriteIndex;

	if (data.m_lineX == 0)
	{
		while (data.m_spritePrefetchLine < SPRITE_SINGLE_SIZE)
		{
			if (GetCurrentSprite(currentSpriteIndex, SPRITE_SINGLE_SIZE - data.m_spritePrefetchLine))
			{
				data.m_spriteFetcher.SetSpriteAttributes(&data.m_lineSprites[currentSpriteIndex]);
				bool fetchFinished = data.m_spriteFetcher.Step(data.m_lineX, data.m_lineY, data.m_spriteFIFO, processedCycles, memory);
				if (fetchFinished)
				{
					data.m_lineSpriteMask |= (1 << currentSpriteIndex);
				}
				return;
			}
			data.m_spritePrefetchLine++;
		}
	}

	if (GetCurrentSprite(currentSpriteIndex, 8))
	{
		data.m_spriteFetcher.SetSpriteAttributes(&data.m_lineSprites[currentSpriteIndex]);
		bool fetchFinished = data.m_spriteFetcher.Step(data.m_lineX, data.m_lineY, data.m_spriteFIFO, processedCycles, memory);
		if (fetchFinished)
		{
			data.m_lineSpriteMask |= (1 << currentSpriteIndex);
		}
		return;
	}
	else
	{
		data.m_backgroundFetcher.Step(data.m_lineX, data.m_lineY, data.m_backgroundFIFO, processedCycles, memory);
	}
	
	if (data.m_lineX == 0)
	{
		if (data.m_fineScrollX > 0 && data.m_backgroundFIFO.Size() > SPRITE_SINGLE_SIZE)
		{
			
			for (uint8_t i = 0; i < 2; ++i)
			{

				data.m_backgroundFIFO.Pop();
				data.m_fineScrollX--;
				if (data.m_fineScrollX == 0)
				{
					break;
				}
			}

			return;
		}
	}

	if (data.m_backgroundFIFO.Size() > SPRITE_SINGLE_SIZE )
	{
		RenderNextPixel(memory);
		RenderNextPixel(memory);
	}
	else
	{
		int x = 0;
	}
}

void PPU::RenderNextPixel(Memory& memory)
{
	Pixel bgPixel = data.m_backgroundFIFO.Pop();

	RGBA pixelColor = data.m_cachedBackgroundColors[bgPixel.m_color];

	if (!data.m_cachedBackgroundEnabled && data.m_windowState != WindowState::Draw)
	{
		pixelColor = SCREEN_COLORS[0];
	}

	if (data.m_spriteFIFO.Size() > 0)
	{
		Pixel spritePixel = data.m_spriteFIFO.Pop();
		RGBA spritePixelColor = PPUHelpers::ResolvePixelColor(spritePixel.m_color, spritePixel.m_palette == 0 ? OBJ0_REGISTER : OBJ1_REGISTER, memory);

		if (PPUHelpers::IsControlFlagSet(LCDControlFlags::ObjEnable, memory) && 
			(spritePixel.m_color != 0 && (!spritePixel.m_backgroundPriority || pixelColor == SCREEN_COLORS[0])))
		{
			pixelColor = spritePixelColor;
		}
	}

	uint32_t renderIndex = data.m_lineX + data.m_lineY * EmulatorConstants::SCREEN_WIDTH;
	m_activeFrame[renderIndex] = pixelColor;
	data.m_lineX++;
}

void PPU::ScanOAM(const uint32_t& positionInLine, Memory& memory)
{
	uint8_t oamEntry = (positionInLine / 2);

	SpriteAttributes attr = memory.ReadOAMEntry(oamEntry);
	bool doubleSize = PPUHelpers::IsControlFlagSet(LCDControlFlags::ObjSize, memory);
	uint8_t doubleSizeAdjustment = doubleSize ? 0 : SPRITE_SINGLE_SIZE;

	bool isInLine = data.m_lineY + doubleSizeAdjustment < attr.m_posY && data.m_lineY + SPRITE_DOUBLE_SIZE >= attr.m_posY;

	if (data.m_lineSpriteCount < MAX_SPRITES_PER_LINE && isInLine)
	{
		data.m_lineSprites[data.m_lineSpriteCount] = attr;
		data.m_lineSpriteCount++;
	}
}

bool PPU::GetCurrentSprite(uint8_t& spriteIndex, uint8_t offset)
{
	for (uint8_t i = 0; i < data.m_lineSpriteCount; ++i)
	{
		int16_t posDifference = data.m_lineSprites[i].m_posX - (data.m_lineX + offset);
		bool isAvailable = (data.m_lineSpriteMask & (1 << i)) == 0;
		if (isAvailable && (posDifference == 0 || posDifference == 1))
		{
			spriteIndex = i;
			return true;
		}
	}
	return false;
}

void PPU::CacheBackgroundPalette(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue, void* userData)
{
	PPU* ppu = reinterpret_cast<PPU*>(userData);
	for (uint32_t i = 0; i < 4; ++i)
	{
		uint32_t colorIndex = (newValue >> (i * 2)) & 0x3;
		ppu->data.m_cachedBackgroundColors[i] = SCREEN_COLORS[colorIndex];
	}
}

void PPU::LCDCWrite(Memory* memory, uint16_t addr, uint8_t prevValue, uint8_t newValue, void* userData)
{
	PPU* ppu = reinterpret_cast<PPU*>(userData);
	ppu->data.m_cachedBackgroundEnabled = (newValue & (1 << static_cast<uint8_t>(LCDControlFlags::BgEnable))) > 0;


	bool PPUPowerPrev = (prevValue & (1 << static_cast<uint8_t>(LCDControlFlags::LCDEnable))) > 0;
	bool PPUPowerNew = (newValue & (1 << static_cast<uint8_t>(LCDControlFlags::LCDEnable))) > 0;
	if (PPUPowerPrev && !PPUPowerNew)
	{
		ppu->DisableScreen(*memory);
	}
	else if (!PPUPowerPrev && PPUPowerNew)
	{
		ppu->TransitionToOAMScan(*memory);
	}
}

void PPU::Serialize(uint8_t* sData)
{
	uint32_t dataSize = sizeof(data);
	WriteAndMove(sData, &data, dataSize);
}

void PPU::Deserialize(const uint8_t* sData)
{
	ReadAndMove(sData, &data, sizeof(data));
}

uint32_t PPU::GetSerializationSize()
{
	return sizeof(data);
}
