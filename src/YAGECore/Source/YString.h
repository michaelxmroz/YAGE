#pragma once

#include "CppIncludes.h"
#include "Logging.h"

#define DEFAULT_FIXED_STRING_SIZE 256

template<size_t SIZE>
class FixedString
{
public:
	explicit FixedString(const char* init = "")
	{
		Assign(init);
	}

	const char* c_str() const
	{
		return m_buffer;
	}

	size_t Length() const
	{
		return m_length;
	}

	void Assign(const char* ptr)
	{
		if (ptr)
		{
			m_length = std::strlen(ptr);

			if (m_length >= SIZE)
			{
				LOG_ERROR("Surpassed max length of fixed string, truncating.");
				m_length = SIZE - 1;
			}

			memcpy(m_buffer, ptr, m_length);
			m_buffer[m_length] = '\0';
		}
	}
	operator const char* () const
	{
		return m_buffer;
	}

private:
	size_t m_length{0};
	char m_buffer[SIZE];
};

typedef FixedString<DEFAULT_FIXED_STRING_SIZE> yString;