#include "../Include/Emulator.h"
#include "VirtualMachine.h"

Emulator* Emulator::Create()
{
	return new VirtualMachine();
}

void Emulator::Delete(Emulator* emulator)
{
	delete emulator;
}

Emulator::~Emulator()
{
}