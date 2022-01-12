#pragma once
#include <vector>
#include "Registers.h"
#include "CPU.h"
#include "Memory.h"
#include "Clock.h"
#include "PPU.h"

class VirtualMachine
{
public:
	VirtualMachine();

	bool Load(std::shared_ptr<std::vector<char>> romBlob);

	bool Start();

private:

	std::shared_ptr<std::vector<char>> m_romBlob;

	Memory m_memory;
	CPU m_cpu;
	Clock m_clock;
	PPU m_ppu;
};

