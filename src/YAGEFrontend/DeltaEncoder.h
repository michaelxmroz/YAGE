#pragma once

#include "FixedSizeRingbuffer.h"
#include <cstdint>
#include <cstring>

constexpr size_t DELTA_FRAME_MAX_SIZE = 1024 * 4; // 4KB max size for delta frames
constexpr size_t DELTA_FRAME_MAX_FRAMES = 2500;

struct FixedSizeDeltaFrame
{
	uint64_t m_data[DELTA_FRAME_MAX_SIZE];
	uint32_t m_size;
};

class DeltaEncoder
{
public:
	void EncodeFrameDelta(const uint8_t* currentFrameData, uint32_t size, const FixedSizeDeltaFrame* previousFrame, FixedSizeDeltaFrame& delta)
	{
		if (previousFrame->m_size == 0) // First frame
		{
			LOG_ERROR("DeltaEncoder: Previous frame size is zero, cannot encode delta.");
			return;
		}

		
		for (uint32_t i = 0; i < size; i++)
		{
			if (currentFrameData[i] != previousFrame->m_data[i])
			{
				delta.m_data[i] = currentFrameData[i] ^ previousFrame->m_data[i];
			}
		}
		delta.m_size = size;
	}

	void DecodeFrameDelta(const FixedSizeDeltaFrame& delta, FixedSizeDeltaFrame& frame)
	{
		for (uint32_t i = 0; i < delta.m_size; i++)
		{
			frame.m_data[i] ^= delta.m_data[i];
		}
	}
}; 