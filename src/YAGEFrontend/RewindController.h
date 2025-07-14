#pragma once
#include <cstdint>
#include <memory>

class DeltaEncoder;
class FixedSizeRingbuffer;

class RewindController
{
public:
	RewindController();
	~RewindController();

	// TODO: Implement rewind controller functionality
	// This class will coordinate the rewind system,
	// managing state capture, storage, and restoration

private:
	// TODO: Add member variables for rewind controller
	std::unique_ptr<DeltaEncoder> m_deltaEncoder;
	// TODO: Add ring buffer for state storage
}; 