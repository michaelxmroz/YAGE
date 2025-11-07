#pragma once
#include <cstdint>
#include <memory>
#include "DeltaEncoder.h"
#include "FixedSizeRingbuffer.h"

class RewindController
{
public:
	RewindController()
	{
		m_deltaEncoder = std::make_unique<DeltaEncoder>();
		m_deltaBuffer = new FixedSizeRingbuffer<FixedSizeDeltaFrame>(DELTA_FRAME_MAX_FRAMES);
		m_previousFrame = new FixedSizeDeltaFrame();
	}
	~RewindController()
	{
		delete m_deltaBuffer;
		delete m_previousFrame;
	}

	RewindController(const RewindController& other)
	{
		m_deltaBuffer = new FixedSizeRingbuffer<FixedSizeDeltaFrame>(*(other.m_deltaBuffer));
		m_previousFrame = new FixedSizeDeltaFrame(*(other.m_previousFrame));
		m_deltaEncoder = std::make_unique<DeltaEncoder>();
	}

	RewindController& operator=(const RewindController& other)
	{
		if (this != &other)
		{
			delete m_deltaBuffer;
			delete m_previousFrame;
			m_deltaBuffer = new FixedSizeRingbuffer<FixedSizeDeltaFrame>(*(other.m_deltaBuffer));
			m_previousFrame = new FixedSizeDeltaFrame(*(other.m_previousFrame));
		}
		return *this;
	}

	bool Rewind(SerializationView& reconstructedFrame)
	{
		const FixedSizeDeltaFrame* delta = m_deltaBuffer->Pop();
		if (delta == nullptr)
		{
			return false; // No more frames to rewind
		}

		m_deltaEncoder->DecodeFrameDelta(*delta, *m_previousFrame);

		reconstructedFrame.data = reinterpret_cast<uint8_t*>(m_previousFrame->m_data);
		reconstructedFrame.size = m_previousFrame->m_size;

		return true;
	}

	void EncodeFrameDelta(SerializationView& currentFrameData)
	{
		if (m_previousFrame->m_size != 0)
		{
			FixedSizeDeltaFrame& delta = m_deltaBuffer->Push();

			m_deltaEncoder->EncodeFrameDelta(currentFrameData.data, currentFrameData.size, m_previousFrame, delta);
		}

		m_previousFrame->m_size = currentFrameData.size;
		memcpy(m_previousFrame->m_data, currentFrameData.data, currentFrameData.size);
	}

private:
	std::unique_ptr<DeltaEncoder> m_deltaEncoder;
	FixedSizeRingbuffer<FixedSizeDeltaFrame>* m_deltaBuffer;
	FixedSizeDeltaFrame* m_previousFrame;
}; 