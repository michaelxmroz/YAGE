#pragma once
#include <algorithm>

template<typename T>
class FixedSizeRingbuffer
{
public:
	FixedSizeRingbuffer(size_t maxSize)
	{
		m_maxSize = maxSize;
		m_buffer = new T[m_maxSize];
		m_head = 0;
		m_tail = 0;
		m_count = 0;
	}

	~FixedSizeRingbuffer()
	{
		delete[] m_buffer;
	}

	FixedSizeRingbuffer(const FixedSizeRingbuffer& other)
	{
		m_maxSize = other.m_maxSize;
		m_buffer = new T[m_maxSize];
		memcpy(m_buffer, other.m_buffer, m_maxSize * sizeof(T));
		m_head = other.m_head;
		m_tail = other.m_tail;
		m_count = other.m_count;
	}
	FixedSizeRingbuffer& operator=(const FixedSizeRingbuffer& other)
	{
		if (this != &other)
		{
			delete[] m_buffer;
			m_maxSize = other.m_maxSize;
			m_buffer = new T[m_maxSize];
			memcpy(m_buffer, other.m_buffer, m_maxSize * sizeof(T));
			m_head = other.m_head;
			m_tail = other.m_tail;
			m_count = other.m_count;
		}
		return *this;
	}

	void Push(const T& item)
	{
		if (m_head == m_tail)
		{
			// Buffer is full, move the tail forward
			m_tail = (m_tail + 1) % m_maxSize;
		}
		m_buffer[m_head] = item;
		m_head = (m_head + 1) % m_maxSize;
		m_count = std::min(m_count + 1, m_maxSize);	
	}
	T& Push()
	{
		if (m_head == m_tail)
		{
			// Buffer is full, move the tail forward
			m_tail = (m_tail + 1) % m_maxSize;
		}
		T& ret = m_buffer[m_head];
		m_head = (m_head + 1) % m_maxSize;
		m_count = std::min(m_count + 1, m_maxSize);	
		return ret;
	}

	bool IsFull() const
	{
		return m_count == m_maxSize;
	}

	const T* First() const
	{
		if (IsEmpty())
		{
			return nullptr;
		}
		return m_buffer + m_tail;
	}

	const T* Pop()
	{
		if (IsEmpty())
		{
			return nullptr;
		}
		m_head = (m_head + m_maxSize - 1) % m_maxSize;
		m_count--;
		return m_buffer + m_head;
	}

	bool IsEmpty() const
	{
		return m_count == 0;
	}

	void Clear()
	{
		m_head = 0;
		m_tail = 0;
		m_count = 0;
	}

	size_t Capacity() const
	{
		return m_maxSize;
	}
private:
	T* m_buffer;
	size_t m_maxSize;
	size_t m_head;
	size_t m_tail;
	size_t m_count;
}; 