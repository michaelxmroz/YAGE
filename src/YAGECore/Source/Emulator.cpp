#include "../Include/Emulator.h"
#include "Allocator.h"
#include "VirtualMachine.h"

Emulator* Emulator::Create(YAGEAllocFunc allocFunc, YAGEFreeFunc freeFunc)
{
	Allocator::Initialize(allocFunc, freeFunc);
	return Y_NEW(VirtualMachine);
}

void Emulator::Delete(Emulator* emulator)
{
	Y_DELETE(emulator);
	Allocator::FreeAll();
}

Emulator::~Emulator()
{
}

uint32_t Emulator::GetMemoryUse() const
{
	return Allocator::GetInstance().GetMemoryUse();
}
