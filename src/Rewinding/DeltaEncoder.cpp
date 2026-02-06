#include "DeltaEncoder.h"

namespace
{
	void UpdateStats(size_t newValue, CompressionStats& stats)
	{
		stats.m_count++;

		if (newValue == 0)
		{
			stats.m_spilled++;
		}
		else
		{
			stats.m_avg += ((static_cast<float>(newValue) - stats.m_avg) / static_cast<float>(stats.m_count));
			stats.m_max = std::max(stats.m_max, newValue);
			stats.m_min = std::min(stats.m_min, newValue);
		}
	}
}

FixedSizeDeltaFrame deltaDecompressionCache;

void DeltaEncoder::EncodeFrameDelta(const uint8_t* currentFrameData, uint64_t size, const FixedSizeDeltaFrame* previousFrame, FixedSizeDeltaFrame& delta, CompressionStats& stats)
{
	if (previousFrame->m_size == 0) // First frame
	{
		return;
	}
	ProgressiveRLEEncoder compressor;

	
	/*
	for (uint64_t i = 0; i < size; i++)
	{
		compressor.Encode(currentFrameData[i] ^ previousFrame->m_data[i], delta);
	}
	*/
	deltaDecompressionCache.m_size = size;

	for (uint64_t i = 0; i < size; i++)
	{
		deltaDecompressionCache.m_data[i] = currentFrameData[i] ^ previousFrame->m_data[i];
	}

	delta.m_size = compressor.Encode(deltaDecompressionCache, delta.m_data);

	UpdateStats(delta.m_size, stats);
	if (delta.m_size == 0)
	{
		LOG_ERROR("compressed frame over capacity");
	}
	/*
	if (delta.m_size == 0)
	{
		delta.m_data[0] = static_cast<uint8_t>(CompressionTag::HANDLE);
		m_spillBuffer.emplace_back({ delta.m_data[0] });
		delta.m_size = sizeof;
	}
	else
	{

	}
	*/
	//compressor.Finalize(delta);
	//delta.m_size = compressor.GetEncodedSize();


}

/*
void DeltaEncoder::EncodeFrameDeltaWithDecompress(const uint8_t* currentFrameDataCompressed, uint64_t size, const FixedSizeDeltaFrame* previousFrame, FixedSizeDeltaFrame& delta)
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
*/
void DeltaEncoder::DecodeFrameDelta(const FixedSizeDeltaFrame& delta, FixedSizeDeltaFrame& frame)
{
	ProgressiveRLEDecoder compressor;
	deltaDecompressionCache.m_size = compressor.Decode(delta, deltaDecompressionCache);
	for (uint64_t i = 0; i < deltaDecompressionCache.m_size; i++)
	{
		frame.m_data[i] ^= deltaDecompressionCache.m_data[i];
	}
}

void DeltaEncoder::Reset()
{
	m_spillBuffer.clear();
	m_spillBuffer.reserve(10);
}
