#pragma once

#include "Allocator.h"
#include "Logging.h"

template<typename T>
class yVector
{
public:
	yVector()
	{
	}

	yVector(uint32_t size)
	{
		reserve(size);
	}

	~yVector()
	{
		Y_DELETE_A(m_buffer);
	}

	yVector(const yVector& other) = delete;
	yVector& operator=(const yVector& other) = delete;
	yVector(yVector&& other) = delete;
	yVector& operator=(yVector&& other) = delete;

	void push_back(const T& value)
	{
		if(m_count + 1 >= m_reservedSize)
		{
			LOG_ERROR("Trying to add to vector past its max capacity");
			return;
		}
		m_buffer[m_count] = value;
		++m_count;
	}

	uint32_t size() const
	{
		return m_count;
	}

	void resize(uint32_t size)
	{
		reserve(size);
		m_count = m_reservedSize;
	}

	void reserve(uint32_t size)
	{
		if (size == 0)
		{
			return;
		}

		if (m_buffer != nullptr && size != m_reservedSize)
		{
			LOG_ERROR("Trying to allocate yVector more than once");
			return;
		}
		m_buffer = Y_NEW_A(T, size);
		m_reservedSize = size;
	}

	T* data()
	{
		return m_buffer;
	}

private:
	T* m_buffer{ nullptr };
	uint32_t m_reservedSize{ 0 };
	uint32_t m_count{ 0 };
};