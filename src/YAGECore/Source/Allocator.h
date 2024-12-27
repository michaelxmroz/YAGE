#pragma once
#include <numeric>
#include "../Include/Emulator.h"

#ifndef _DEBUG
// In release we request about 10mb of memory, this should be more than enough. 
// In case it is not some big asserts are going to trigger and this number needs to be bumped.
#define INITIAL_MEMORY_REQUEST 0xA00000
#else // !_DEBUG
// In debug we get 167mb, which again, should be plenty for the additional debug info we are logging.
#define INITIAL_MEMORY_REQUEST 0xA000000
#endif

class Allocator
{
public:
	static void Initialize(YAGEAllocFunc allocFunc, YAGEFreeFunc freeFunc)
	{
		Allocator& instance = GetInstance();
		instance.m_allocFunc = allocFunc;
		instance.m_freeFunc = freeFunc;
		instance.m_buffer = reinterpret_cast<uint8_t*>(allocFunc(INITIAL_MEMORY_REQUEST));
		instance.m_nextFree = instance.m_buffer;
		instance.m_allocatedSize = 0;
	}

	static void FreeAll()
	{
		Allocator& instance = GetInstance();
		if (instance.m_buffer)
		{
			instance.m_freeFunc(instance.m_buffer);
			instance.m_buffer = nullptr;
		}
	}

	static Allocator& GetInstance() 
	{
		static Allocator instance;
		return instance;
	}

	static void* Malloc(uint32_t size)
	{
		Allocator& instance = GetInstance();
#ifdef _DEBUG
		// TODO log & assert if memory buffer has not been initialized ( =nullptr)
#endif
		instance.m_allocatedSize += size;

		if (instance.m_allocatedSize >= instance.m_bufferCapacity)
		{
			// TODO log & assert that we have exceeded our pre-allocated capacity
		}

		void* returnAddr = instance.m_nextFree;

		instance.m_nextFree += size;

		return returnAddr;
	}

	static void Free(void* ptr)
	{
		//As this is a linear allocator, nothing to do here.
		//TODO initialization count for leak detection
	}

	uint32_t GetMemoryUse()
	{
		return m_allocatedSize;
	}

private:
	Allocator() = default;
	~Allocator()
	{
		m_freeFunc(m_buffer);
	}

	Allocator(const Allocator&) = delete;
	Allocator& operator=(const Allocator&) = delete;
	Allocator(Allocator&&) = delete;
	Allocator& operator=(Allocator&&) = delete;

	uint8_t* m_buffer = nullptr;
	const uint32_t m_bufferCapacity = INITIAL_MEMORY_REQUEST;
	uint8_t* m_nextFree = nullptr;
	uint32_t m_allocatedSize = 0;
	
	YAGEAllocFunc m_allocFunc = nullptr;
	YAGEFreeFunc m_freeFunc = nullptr;
};

// Custom allocation and initialization function
template <typename T, typename... Args>
T* YAGENew(Args&&... args)
{
	void* memory = Allocator::Malloc(sizeof(T));
	if (!memory)
	{
		throw std::bad_alloc(); // Handle allocation failure
	}

	//TODO LOG ALLOCATION

	return new (memory) T(std::forward<Args>(args)...); // Perfect forwarding of arguments
}

template <typename T>
void YAGEDelete(T* ptr)
{
	if (ptr)
	{
		ptr->~T();
		Allocator::Free(ptr);
	}
}

template <typename T>
T* YAGENewA(size_t count)
{
	void* memory = Allocator::Malloc(sizeof(T) * static_cast<uint32_t>(count) + sizeof(uint32_t));
	if (!memory)
	{
		throw std::bad_alloc(); // Handle allocation failure
	}

	//TODO LOG ALLOCATION
	uint32_t* countStore = reinterpret_cast<uint32_t*>(memory);
	*countStore = static_cast<uint32_t>(count);

	countStore++;
	T* firstEntry = reinterpret_cast<T*>(countStore);
	T* entry = firstEntry;

	for (uint32_t i = 0; i < count; ++i)
	{
		new (entry) T();
		entry++;
	}
	
	return firstEntry;
}

template <typename T>
typename std::enable_if<!std::is_trivially_destructible<T>::value>::type
YAGEDeleteA(T* ptr)
{
	if (ptr)
	{
		uint32_t* countPtr = reinterpret_cast<uint32_t*>(ptr)-1;
		uint32_t count = *countPtr;

		for (uint32_t i = 0; i < count; ++i)
		{
			uint32_t revIndex = count - i - 1;
			(ptr + revIndex)->~T();
		}

		Allocator::Free(countPtr);
	}
}

template <typename T>
typename std::enable_if<std::is_trivially_destructible<T>::value>::type
YAGEDeleteA(T* ptr)
{
	if (ptr)
	{
		uint32_t* countPtr = reinterpret_cast<uint32_t*>(ptr) - 1;

		Allocator::Free(countPtr);
	}
}

#define Y_NEW(type, ...) YAGENew<type>(__VA_ARGS__)
#define Y_DELETE(ptr) YAGEDelete(ptr)

#define Y_NEW_A(type, count) YAGENewA<type>(count)
#define Y_DELETE_A(ptr) YAGEDeleteA(ptr)
