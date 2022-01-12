#pragma once
#include "Memory.h"

#define MAX_SPRITES_PER_LINE 10

class PPU
{
public:
	PPU();

	void Init(Memory& memory);

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

	void Render(uint32_t mCycles, Memory& memory);

private:

	enum class SpriteFlags
	{
		PaletteNr = 4,
		XFlip = 5,
		YFlip = 6,
		BgOverObj = 7,
	};

	void CheckForNewScanline(uint32_t totalCycles, Memory& memory);

	bool IsControlFlagSet(ControlFlags flag, Memory& memory) const;
	bool IsStatFlagSet(StatFlags flag, Memory& memory) const;
	void SetStatFlag(StatFlags flag, Memory& memory) const;
	void ResetStatFlag(StatFlags flag, Memory& memory) const;
	void SetModeFlag(uint8_t mode, Memory& memory) const;
	bool IsSpriteFlagSet(SpriteFlags flag, uint8_t flags) const;

	uint32_t m_totalCycles;
	uint8_t m_lineY;
	uint8_t m_lineSpriteCount;
	SpriteAttributes m_lineSprites[MAX_SPRITES_PER_LINE];
};

