#pragma once

#include "Debug.h"
#include "SmallVector.h"

#include <vector>
#include <any>

#define TRACK_STACK_ALLOCATIONS DEBUG

#if defined(TRACK_STACK_ALLOCATIONS)
#include <typeinfo>
#endif

template<size_t k_buffer_size = 4096>
class StackAllocator
{
private:
#if defined(TRACK_STACK_ALLOCATIONS)
	struct TypeAlloc
	{
		TypeAlloc(void* ptr, size_t type, size_t count) : ptr(ptr), type(type), count(count) {}
		void* ptr;
		size_t type;
		size_t count;
	};
#endif

public:
	StackAllocator()
	{
		Clear();
	}

	~StackAllocator()
	{
#if defined(TRACK_STACK_ALLOCATIONS)
		DEBUG_ASSERT(m_allocations.size() == 0, "Not all allocations were freed before freeing the stack allocator");
		DEBUG_ASSERT(m_typed_allocations.size() == 0, "Not all typed allocations were freed before freeing the stack allocator");
#endif
	}

	void* Alloc(size_t size)
	{
		void* ptr = static_cast<void*>(m_offset);

#if defined(TRACK_STACK_ALLOCATIONS)
		if (m_offset + size > m_buffer + k_buffer_size)
		{
			DEBUG_PRINT_ERROR("We are trying to allocate more than we can hold");
			return nullptr;
		}

		m_allocations.push_back(ptr);
#endif

		m_offset += size;
		return ptr;
	}

	void Free(void* ptr)
	{
		DEBUG_ASSERT(ptr != nullptr, "We can't free a nullptr");

#if defined(TRACK_STACK_ALLOCATIONS)
		auto it = std::remove(m_allocations.begin(), m_allocations.end(), ptr);
		size_t r = m_allocations.end() - it;

		DEBUG_ASSERT(r == 1, "We tried to free a pointer that was not allocated from us");

		m_allocations.erase(it, m_allocations.end());
#endif
	}

	void Clear()
	{
		m_offset = m_buffer;

#if defined(TRACK_STACK_ALLOCATIONS)
		m_allocations.clear();

		DEBUG_ASSERT(m_typed_allocations.size() == 0, "Typed allocations can't be cleared so must be manually freed");
#endif
	}

	template<class T>
	T* New()
	{
		return AllocArray<T>(1);
	}

	template<class T>
	void Delete(T* ptr)
	{
		FreeArray(ptr, 1);
	}

	template<class T>
	T* NewArray(size_t count)
	{
		void* ptr = Alloc(sizeof(T) * count);

#if defined(TRACK_STACK_ALLOCATIONS)
		m_typed_allocations.emplace_back(ptr, typeid(T).hash_code(), count);
#endif

		for (size_t i = 0; i < count; i++)
		{
			new (static_cast<T*>(ptr) + i) T();
		}

		return static_cast<T*>(ptr);
	}

	template<class T>
	void DeleteArray(T* ptr, size_t count)
	{
		DEBUG_ASSERT(ptr != nullptr, "We can't destroy a nullptr");

		std::destroy_n(ptr, count);

#if defined(TRACK_STACK_ALLOCATIONS)
		auto find_func = [ptr, count](TypeAlloc& alloc)
		{
			return alloc.ptr == ptr && alloc.type == typeid(T).hash_code() && alloc.count == count;
		};

		auto it = std::remove_if(m_typed_allocations.begin(), m_typed_allocations.end(), find_func);
		size_t r = m_typed_allocations.end() - it;

		DEBUG_ASSERT(r == 1, "We tried to free a pointer that was not allocated from us");

		m_typed_allocations.erase(it, m_typed_allocations.end());
#endif

		Free(ptr);
	}

private:
	std::byte* m_offset;

#if defined(TRACK_STACK_ALLOCATIONS)
	GrowingSmallVector<void*, 32> m_allocations;
	GrowingSmallVector<TypeAlloc, 16> m_typed_allocations;
#endif

	std::byte m_buffer[k_buffer_size];
};