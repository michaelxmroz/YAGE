#pragma once

#include "FixedSizeRingbuffer.h"
#include <cstdint>
#include <cstring>

constexpr size_t DELTA_FRAME_MAX_SIZE_UNCOMPRESSED = 1024 * 60; // 50KB max size for uncompressed frames
constexpr size_t DELTA_FRAME_MAX_SIZE = 1024 * 4; // 4KB max size for delta frames
constexpr size_t DELTA_FRAME_MAX_FRAMES_L1 = 60;
constexpr size_t DELTA_FRAME_MAX_FRAMES_L2 = 60;

struct FixedSizeDeltaFrame
{
	uint8_t m_data[DELTA_FRAME_MAX_SIZE_UNCOMPRESSED];
	uint64_t m_size;
};

struct CompressedDeltaFrame
{
	uint8_t m_data[DELTA_FRAME_MAX_SIZE];
	uint64_t m_size;
};

class DeltaEncoder
{
public:
	void EncodeFrameDelta(const uint8_t* currentFrameData, uint64_t size, const FixedSizeDeltaFrame* previousFrame, FixedSizeDeltaFrame& delta);

	void DecodeFrameDelta(const FixedSizeDeltaFrame& delta, FixedSizeDeltaFrame& frame);
}; 