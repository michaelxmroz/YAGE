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
	return frameNumber % m_rewindDataSets.front().m_frequency == 0;
}

void RewindController::EncodeFrameDelta(uint64_t frameNumber, const SerializationView& currentFrameData, CompressionStats& stats)
{
	auto dataSetIt = m_rewindDataSets.begin();
	const RewindData& firstDataSet = *dataSetIt;
	// Populate the frame cache structure on the first frame
	if (firstDataSet.m_previousFrame->m_size == 0)
	{
		for (const RewindData& dataSet : m_rewindDataSets)
		{
			dataSet.m_previousFrame->m_size = currentFrameData.size;
			memcpy(dataSet.m_previousFrame->m_data, currentFrameData.data, currentFrameData.size);

			dataSet.m_cachedDelta->m_size = currentFrameData.size;
			memcpy(dataSet.m_cachedDelta->m_data, currentFrameData.data, currentFrameData.size);
		}
		return;
	}

	uint8_t* deltaDataPtr = currentFrameData.data;
	uint64_t deltaSize = currentFrameData.size;

	while (dataSetIt != m_rewindDataSets.end())
	{
		const RewindData& dataSet = *dataSetIt;
		if (frameNumber % dataSet.m_frequency == 0)
		{
			// We are overflowing the current cache, so get the last entry and push that into the next highest tier in the hierarchy
			bool needHigherLevelUpdate = dataSet.m_deltaBuffer->IsFull();
					
			if (needHigherLevelUpdate)
			{
				const FixedSizeDeltaFrame* evictedDelta = dataSet.m_deltaBuffer->First();
				dataSetIt++;
				const RewindData& nextDataSet = *dataSetIt;
				// reconstruct the next frame from the cached delta and the evicted delta and save it it the cached delta
				m_deltaEncoder->DecodeFrameDelta(*evictedDelta, *nextDataSet.m_cachedDelta);
				//m_deltaEncoder->EncodeFrameDeltaWithDecompress(evictedDelta->m_data, evictedDelta->m_size, nextDataSet.m_cachedDelta, *nextDataSet.m_cachedDelta, stats);
			}
			else
			{
				dataSetIt = m_rewindDataSets.end();
			}

			FixedSizeDeltaFrame& delta = dataSet.m_deltaBuffer->Push();

			m_deltaEncoder->EncodeFrameDelta(deltaDataPtr, deltaSize, dataSet.m_previousFrame, delta, stats);


			dataSet.m_previousFrame->m_size = deltaSize;
			memcpy(dataSet.m_previousFrame->m_data, deltaDataPtr, deltaSize);

			if (needHigherLevelUpdate)
			{
				const RewindData& nextDataSet = *dataSetIt;
				deltaDataPtr = nextDataSet.m_cachedDelta->m_data;
				deltaSize = nextDataSet.m_cachedDelta->m_size;
			}
		}
		else 
		{
			break;
		}
	}
}
