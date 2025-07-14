#include "RewindController.h"
#include "DeltaEncoder.h"

RewindController::RewindController()
{
	// TODO: Initialize rewind controller
	m_deltaEncoder = std::make_unique<DeltaEncoder>();
}

RewindController::~RewindController()
{
	// TODO: Cleanup rewind controller
} 