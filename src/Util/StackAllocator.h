#pragma once

#include "Debug.h"
#include "SmallVector.h"

#include <vector>
#include <any>

// Enable tracking of allocations to check nothing is being misused
#if DEBUG
#define TRACK_STACK_ALLOCATIONS
#endif

#if defined(TRACK_STACK_ALLOCATIONS)
#include <typeinfo>
#endif

// A memory and object allocator that has a constant size buffer and will allocate objects from that buffer.
// Best used when constructed on the stack to construct multiple temporary objects with runtime sizes.
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

	// Allocate some aligned bytes from the buffer
	void* Alloc(size_t size, size_t align)
	{
		std::intptr_t ptr = reinterpret_cast<std::intptr_t>(m_offset);
		
		std::intptr_t aligned_ptr = (ptr + align - 1) & ~(align - 1);
		size = aligned_ptr - ptr + size;

#if defined(TRACK_STACK_ALLOCATIONS)
		if (m_offset + size > m_buffer + k_buffer_size)
		{
			DEBUG_PRINT_ERROR("We are trying to allocate more than we can hold");
			return nullptr;
		}

		m_allocations.push_back(reinterpret_cast<void*>(aligned_ptr));
#endif

		m_offset += size;
		return reinterpret_cast<void*>(aligned_ptr);
	}

	// Allocate some unaligned bytes from the buffer
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

	// Free some bytes allocated with the basic Alloc(). In release this doesn't do anything
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

	// Clear all typeless allocations so that new ones can be made if needed
	void Clear()
	{
		m_offset = m_buffer;

#if defined(TRACK_STACK_ALLOCATIONS)
		m_allocations.clear();

		DEBUG_ASSERT(m_typed_allocations.size() == 0, "Typed allocations can't be cleared so must be manually freed");
#endif
	}

	// Allocate a new object that will be default constructed. Its alignment spec will not be followed
	template<class T>
	T* New()
	{
		return AllocArray<T>(1);
	}

	// Delete an object allocated with New(). In release this just destructs the object
	template<class T>
	void Delete(T* ptr)
	{
		FreeArray(ptr, 1);
	}

	// Allocate an array of a type that will be default constructed. Its alignment spec will not be followed
	template<class T>
	T* NewArray(size_t count)
	{
		void* ptr = Alloc(sizeof(T) * count, alignof(T));

#if defined(TRACK_STACK_ALLOCATIONS)
		m_typed_allocations.emplace_back(ptr, typeid(T).hash_code(), count);
#endif

		for (size_t i = 0; i < count; i++)
		{
			new (static_cast<T*>(ptr) + i) T();
		}

		return static_cast<T*>(ptr);
	}

	// Delete an array of objects allocated with NewArray(). In release this just destructs the objects
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