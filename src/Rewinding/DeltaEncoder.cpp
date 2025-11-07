#include "DeltaEncoder.h"

void DeltaEncoder::EncodeFrameDelta(const uint8_t* currentFrameData, uint64_t size, const FixedSizeDeltaFrame* previousFrame, FixedSizeDeltaFrame& delta)
{
	if (previousFrame->m_size == 0) // First frame
	{
		return;
	}

	for (uint64_t i = 0; i < size; i++)
	{
		delta.m_data[i] = currentFrameData[i] ^ previousFrame->m_data[i];
	}
	delta.m_size = size;
}

void DeltaEncoder::DecodeFrameDelta(const FixedSizeDeltaFrame& delta, FixedSizeDeltaFrame& frame)
{
	for (uint64_t i = 0; i < delta.m_size; i++)
	{
		frame.m_data[i] ^= delta.m_data[i];
	}
}
