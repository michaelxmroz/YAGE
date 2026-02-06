#pragma once

#include "FixedSizeRingbuffer.h"
#include <cstdint>
#include <cstring>

#if !_TESTING
#include "../YAGEFrontend/Logger.h"
#endif
#include <vector>

constexpr size_t DELTA_FRAME_MAX_SIZE_UNCOMPRESSED = 1024 * 60; // 50KB max size for uncompressed frames
constexpr size_t DELTA_FRAME_MAX_SIZE = 1024 * 4; // 4KB max size for delta frames

enum class CompressionTag : uint8_t
{
	INVALID = 0,
	ZERO_RUN = 1,
	NON_ZERO_RUN = 2,
	HANDLE = 3
};

struct CompressionStats
{
	float m_avg{ 0 };
	uint64_t m_max{ 0 };
	uint64_t m_min{ std::numeric_limits<uint64_t>::max() };
	uint64_t m_count{ 0 };
	uint64_t m_spilled{ 0 };
};

struct FixedSizeDeltaFrame
{
	uint8_t m_data[DELTA_FRAME_MAX_SIZE_UNCOMPRESSED];
	uint64_t m_size;
};

struct CompressedDeltaFrame
{
	uint8_t m_data[DELTA_FRAME_MAX_SIZE_UNCOMPRESSED];
	uint64_t m_size;
};

struct UncompressedFrameHandle
{
	size_t index;
	FixedSizeDeltaFrame delta;
};

class ProgressiveRLEEncoder
{
public:
	/*
	size_t Encode(const FixedSizeDeltaFrame& frame, uint8_t* compressedData)
	{
		size_t outIndex = 0;
		size_t i = 0;
		while (i < frame.m_size)
		{
			uint8_t val = frame.m_data[i];
			uint8_t count = 1;
			while ((i + count < frame.m_size) && frame.m_data[i + count] == val && count < 255)
			{
				count++;
			}

			if (outIndex >= frame.m_size)
			{
				LOG_ERROR("Compression exceeded buffer size");
				return 0;
			}

			compressedData[outIndex++] = count;
			compressedData[outIndex++] = val;
			i += count;
		}

		return outIndex;
	}
	*/

	size_t Encode(const FixedSizeDeltaFrame& frame, uint8_t* compressedData)
	{
		size_t outIndex = 0;
		size_t i = 0;
		size_t start = 0;
		constexpr size_t int64_SIZE = sizeof(uint64_t);
		while (i < frame.m_size)
		{
			start = i;
			if (frame.m_data[i] == 0)
			{
				while (i + int64_SIZE <= frame.m_size && *reinterpret_cast<const uint64_t*>(frame.m_data + i) == 0)
				{
					i += int64_SIZE;
				}
				while (i < frame.m_size && frame.m_data[i] == 0)
				{
					i++;
				}

				if (outIndex + 1 + sizeof(uint16_t) >= frame.m_size)
				{
#if !_TESTING
					LOG_ERROR("Compression exceeded buffer size");
#endif
					return 0;
				}

				compressedData[outIndex++] = static_cast<uint8_t>(CompressionTag::ZERO_RUN);
				*reinterpret_cast<uint16_t*>(compressedData + outIndex) = static_cast<uint16_t>(i - start);
				outIndex += sizeof(uint16_t);
			}
			else
			{
				uint8_t val = frame.m_data[i];

				while ((i < frame.m_size) && frame.m_data[i] != 0)
				{
					i++;
				}

				if (outIndex + 1 + sizeof(uint16_t) + (i - start) >= frame.m_size)
				{
#if !_TESTING
					LOG_ERROR("Compression exceeded buffer size");
#endif
					return 0;
				}

				compressedData[outIndex++] = static_cast<uint8_t>(CompressionTag::NON_ZERO_RUN);
				*reinterpret_cast<uint16_t*>(compressedData + outIndex) = static_cast<uint16_t>(i - start);
				outIndex += sizeof(uint16_t);

				memcpy(compressedData + outIndex, frame.m_data + start, i - start);
				outIndex += i - start;
			}


		}

		return outIndex;
	}

	void Encode(uint8_t newValue, FixedSizeDeltaFrame& compressedData)
	{
		if (m_count == 0)
		{
			m_currentValue = newValue;
			m_count = 1;
			return;
		}
	
		if (newValue == m_currentValue && m_count < 128)
		{
			m_count++;
			return;
		}
		
		if (m_count >= 3)
		{
			Finalize(compressedData);
		}


		m_currentValue = newValue;
		m_count = 1;
	}

	void Finalize(FixedSizeDeltaFrame& compressedData)
	{
		if (m_outIndex >= DELTA_FRAME_MAX_SIZE_UNCOMPRESSED)
		{
#if !_TESTING
			LOG_ERROR("Compression exceeded buffer size");
#endif
			return;
		}

		compressedData.m_data[m_outIndex++] = 127 + m_count;
		compressedData.m_data[m_outIndex++] = m_currentValue;
	}

	size_t GetEncodedSize()
	{
		return m_outIndex;
	}
	
private:
	size_t m_outIndex{ 0 };
	uint8_t m_count{ 0 };
	uint8_t m_currentValue{ 0 };
};

class ProgressiveRLEDecoder
{
public:
	size_t Decode(const FixedSizeDeltaFrame& compressedFrame, FixedSizeDeltaFrame& decompressedFrame)
	{
		size_t outIndex = 0;
		size_t i = 0;
		while (i < compressedFrame.m_size)
		{
			uint8_t count = compressedFrame.m_data[i++];
			uint8_t val = compressedFrame.m_data[i++];
			memset(decompressedFrame.m_data + outIndex, val, count);
			outIndex += count;

			if (outIndex >= DELTA_FRAME_MAX_SIZE_UNCOMPRESSED)
			{
#if !_TESTING
				LOG_ERROR("Decompression exceeded buffer size");
#endif
				return 0;
			}
		}
		return outIndex;
	}


};

class DeltaEncoder
{
public:
	void EncodeFrameDelta(const uint8_t* currentFrameData, uint64_t size, const FixedSizeDeltaFrame* previousFrame, FixedSizeDeltaFrame& delta, CompressionStats& stats);

	//void EncodeFrameDeltaWithDecompress(const uint8_t* currentFrameDataCompressed, uint64_t size, const FixedSizeDeltaFrame* previousFrame, FixedSizeDeltaFrame& delta, CompressionStats& stats);

	void DecodeFrameDelta(const FixedSizeDeltaFrame& delta, FixedSizeDeltaFrame& frame);
	void Reset();
private:
	std::vector<UncompressedFrameHandle> m_spillBuffer;
}; 