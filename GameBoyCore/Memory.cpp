#include "Memory.h"
#include "Clock.h"

Memory::Memory()
{
	m_memory = new uint8_t[MEMORY_SIZE];
	m_externalMemory = false;
}

Memory::Memory(uint8_t* rawMemory)
{
	m_memory = rawMemory;
	m_externalMemory = true;
}

Memory::~Memory()
{
	if (!m_externalMemory)
	{
		delete[] m_memory;
	}
}

//TODO this needs to be more like the direct write so we can apply changes after the write ops
uint8_t& Memory::Write(uint16_t index)
{
	Clock::CheckForDividerWrite(index, *this);
	return m_memory[index];
}

void Memory::WriteDirect(uint16_t index, uint8_t value)
{
	m_memory[index] = value;
}

void Memory::ClearMemory()
{
	memset(m_memory, 0, MEMORY_SIZE);
}

void Memory::MapROM(std::vector<char>* m_romBlob)
{
	memcpy(m_memory, &((*m_romBlob)[0]), m_romBlob->size());
}
