#pragma once
#include <cstdint>

template <typename T, uint32_t CAPACITY>
class StaticFIFO
{
public:
	StaticFIFO()
	{
		Clear();
	}

	void Push(const T& entry)
	{
		if (m_size == CAPACITY)
		{
			return;
		}

		uint32_t insertIndex = (m_index + m_size) % CAPACITY;
		m_size++;
		m_fifo[insertIndex] = entry;
	}

	const T& Pop()
	{
		T& element = m_fifo[m_index];
		m_index = (m_index + 1) % CAPACITY;
		m_size--;

		return element;
	}

	const T& Get(uint32_t index) const
	{
		return m_fifo[(m_index + index) % CAPACITY];
	}

	const T& Replace(uint32_t index, const T& newElement)
	{
		return m_fifo[(m_index + index) % CAPACITY] = newElement;
	}

	void Clear()
	{
		m_size = 0;
		m_index = 0;
	}

	uint32_t Size() const
	{
		return m_size;
	}

private:
	T m_fifo[CAPACITY];
	uint32_t m_size;
	uint32_t m_index;
};

struct Pixel
{
	uint8_t m_color;
	uint8_t m_palette;
	bool m_backgroundPriority;
};

typedef StaticFIFO<Pixel, 16> PixelFIFO;
