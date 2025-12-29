#pragma once
#include <cstdint>
#include <memory>
#include "DeltaEncoder.h"
#include "FixedSizeRingbuffer.h"
#include "Emulator.h"
#include <vector>


class RewindController
{
public:
	RewindController();

	void Reset();
	SerializationView* Rewind();
	bool ShouldRecordFrame(uint64_t frameNumber);
	void EncodeFrameDelta(uint64_t frameNumber, SerializationView& currentFrameData);

private:

	struct RewindData
	{
		RewindData(uint64_t frequency, uint64_t capacity)
			: m_deltaBuffer(nullptr), m_previousFrame(nullptr), m_cachedDelta(nullptr), m_frequency(frequency)
		{
			m_deltaBuffer = new FixedSizeRingbuffer<FixedSizeDeltaFrame>(capacity);
			m_previousFrame = new FixedSizeDeltaFrame();
			m_cachedDelta = new FixedSizeDeltaFrame();
		}
		~RewindData()
		{
			delete m_deltaBuffer;
			delete m_previousFrame;
			delete m_cachedDelta;
		}
		RewindData(const RewindData& other)
		{
			m_deltaBuffer = new FixedSizeRingbuffer<FixedSizeDeltaFrame>(other.m_deltaBuffer->Capacity());
			*m_deltaBuffer = *other.m_deltaBuffer;

			m_previousFrame = new FixedSizeDeltaFrame();
			m_previousFrame->m_size = other.m_previousFrame->m_size;
			std::memcpy(m_previousFrame->m_data, other.m_previousFrame->m_data, other.m_previousFrame->m_size);	

			m_cachedDelta = new FixedSizeDeltaFrame();
			m_cachedDelta->m_size = other.m_cachedDelta->m_size;
			std::memcpy(m_cachedDelta->m_data, other.m_cachedDelta->m_data, other.m_cachedDelta->m_size);

			m_frequency = other.m_frequency;
		}
		RewindData& operator=(const RewindData& other)
		{
			if (this != &other)
			{
				delete m_deltaBuffer;
				delete m_previousFrame;

				m_deltaBuffer = new FixedSizeRingbuffer<FixedSizeDeltaFrame>(other.m_deltaBuffer->Capacity());
				*m_deltaBuffer = *other.m_deltaBuffer;

				m_previousFrame = new FixedSizeDeltaFrame();
				m_previousFrame->m_size = other.m_previousFrame->m_size;
				std::memcpy(m_previousFrame->m_data, other.m_previousFrame->m_data, other.m_previousFrame->m_size);

				m_cachedDelta = new FixedSizeDeltaFrame();
				m_cachedDelta->m_size = other.m_cachedDelta->m_size;
				std::memcpy(m_cachedDelta->m_data, other.m_cachedDelta->m_data, other.m_cachedDelta->m_size);

				m_frequency = other.m_frequency;
			}
			return *this;
		}

		FixedSizeRingbuffer<FixedSizeDeltaFrame>* m_deltaBuffer;
		FixedSizeDeltaFrame* m_previousFrame;
		FixedSizeDeltaFrame* m_cachedDelta;
		uint64_t m_frequency;
	};

	std::vector<RewindData> m_rewindDataSets{
		RewindData(1, 60),
		RewindData(2, 120),
		 RewindData(60, 2500)
	};

	std::unique_ptr<DeltaEncoder> m_deltaEncoder;
	SerializationView m_reconstructedFrame;
}; 