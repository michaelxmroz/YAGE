#pragma once
#include "Memory.h"
#include "PixelFIFO.h"
#include "PixelFetcher.h"

#define MAX_SPRITES_PER_LINE 10

typedef void (*RenderFunc)(const void* image);

class PPU : ISerializable
{
public:
	PPU(Serializer* serializer);

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

	void ScanOAM(const uint32_t& positionInLine, Memory& memory, uint32_t& processedCycles);
	void RenderNextPixel(Memory& memory);

	void TransitionToVBlank(Memory& memory);
	void TransitionToHBlank(Memory& memory);
	void TransitionToDraw(Memory& memory);
	void TransitionToOAMScan(Memory& memory);

	void DisableScreen(Memory& memory);
	void DrawPixels(Memory& memory, uint32_t& processedCycles);

	bool GetCurrentSprite(uint8_t& spriteIndex, uint8_t offset);

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
			, m_spritePrefetchLine(0)
			, m_lineSpriteMask(0)
			, m_cycleDebt(0)
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
	} data;
	void* m_activeFrame;
	void* m_backBuffer;



	// Inherited via ISerializable
	void Serialize(std::vector<Chunk>& chunks, std::vector<uint8_t>& data) override;
	void Deserialize(const Chunk* chunks, const uint32_t& chunkCount, const uint8_t* data, const uint32_t& dataSize) override;
};

