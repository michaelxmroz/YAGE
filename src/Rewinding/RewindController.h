#pragma once
#include <cstdint>
#include <memory>
#include "DeltaEncoder.h"
#include "FixedSizeRingbuffer.h"
#include "Emulator.h"

class RewindController
{
public:
	RewindController();
	~RewindController();

	RewindController(const RewindController& other);

	RewindController& operator=(const RewindController& other);

	void Reset();

	SerializationView* Rewind();

	void EncodeFrameDelta(SerializationView& currentFrameData);

private:
	std::unique_ptr<DeltaEncoder> m_deltaEncoder;
	FixedSizeRingbuffer<FixedSizeDeltaFrame>* m_deltaBuffer;
	FixedSizeDeltaFrame* m_previousFrame;
	SerializationView m_reconstructedFrame;
}; 