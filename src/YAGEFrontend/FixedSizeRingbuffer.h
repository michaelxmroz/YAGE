#pragma once

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
		}
		return *this;
	}

	// This class will provide a fixed-size circular buffer
	// for storing rewind states efficiently
	void Push(const T& item)
	{
		m_buffer[m_head] = item;
		m_head = (m_head + 1) % m_maxSize;
		if (m_head == m_tail)
		{
			// Buffer is full, move the tail forward
			m_tail = (m_tail + 1) % m_maxSize;
		}
	}
	T& Push()
	{
		T& ret = m_buffer[m_head];
		m_head = (m_head + 1) % m_maxSize;
		if (m_head == m_tail)
		{
			// Buffer is full, move the tail forward
			m_tail = (m_tail + 1) % m_maxSize;
		}
		return ret;
	}
	const T* Pop()
	{
		if (IsEmpty())
		{
			return nullptr;
		}
		m_head = (m_head + m_maxSize - 1) % m_maxSize;
		return m_buffer + m_head;
	}

	bool IsEmpty() const
	{
		return m_head == m_tail;
	}
private:
	T* m_buffer;
	size_t m_maxSize;
	size_t m_head;
	size_t m_tail;
}; 