#include "RewindController.h"
#include "DeltaEncoder.h"

RewindController::RewindController()
{
	m_deltaEncoder = std::make_unique<DeltaEncoder>();

}

void RewindController::Reset()
{
	for (RewindData& dataSet : m_rewindDataSets)
	{
		dataSet.m_deltaBuffer->Clear();
		dataSet.m_previousFrame->m_size = 0;
	}
}

SerializationView* RewindController::Rewind()
{
	for (const RewindData& dataSet : m_rewindDataSets)
	{
		if (!dataSet.m_deltaBuffer->IsEmpty())
		{
			const FixedSizeDeltaFrame* delta = dataSet.m_deltaBuffer->Pop();

			m_deltaEncoder->DecodeFrameDelta(*delta, *dataSet.m_previousFrame);

			m_reconstructedFrame.data = reinterpret_cast<uint8_t*>(dataSet.m_previousFrame->m_data);
			m_reconstructedFrame.size = dataSet.m_previousFrame->m_size;

			return &m_reconstructedFrame;
		}
	}

	return nullptr;
}

bool RewindController::ShouldRecordFrame(uint64_t frameNumber)
{
	for (const RewindData& dataSet : m_rewindDataSets)
	{
		if (frameNumber % dataSet.m_frequency == 0)
		{
			return true;
		}
	}
	return false;
}

void RewindController::EncodeFrameDelta(uint64_t frameNumber, SerializationView& currentFrameData)
{
	for (const RewindData& dataSet : m_rewindDataSets)
	{
		if (frameNumber % dataSet.m_frequency == 0)
		{
			if (dataSet.m_previousFrame->m_size != 0)
			{
				FixedSizeDeltaFrame& delta = dataSet.m_deltaBuffer->Push();

				m_deltaEncoder->EncodeFrameDelta(currentFrameData.data, currentFrameData.size, dataSet.m_previousFrame, delta);
			}

			dataSet.m_previousFrame->m_size = currentFrameData.size;
			memcpy(dataSet.m_previousFrame->m_data, currentFrameData.data, currentFrameData.size);
		}
	}
}
