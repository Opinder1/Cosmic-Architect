#pragma once

#include "Debug.h"
#include "Nocopy.h"

#include <array>
#include <vector>
#include <memory>

#include <robin_hood/robin_hood.h>

#define POLY_DEBUG

// A system for creating runtime defined structs which are efficently allocated in memory
template<class MainT>
class PolyType : Nocopy, Nomove
{
	constexpr static const uint16_t k_invalid_poly_offset = UINT16_MAX;
	constexpr static const size_t k_max_offsets = 16;

public:
	template<class T>
	static const size_t k_type_index;

private:
	using FactoryCB = void (*)(std::byte*);

	struct Entry
	{
		FactoryCB construct = nullptr;
		FactoryCB destruct = nullptr;
		uint16_t offset = 0;
	};

private:
	template<class T>
	static void Construct(std::byte* ptr)
	{
		new (reinterpret_cast<T*>(ptr)) T();
	}

	template<class T>
	static void Destruct(std::byte* ptr)
	{
		std::destroy_at(reinterpret_cast<T*>(ptr));
	}

public:
	PolyType()
	{
		AddType<MainT>();
	}

	~PolyType()
	{
#if defined(POLY_DEBUG)
		DEBUG_ASSERT(m_created.empty(), "We should have destroyed all instances of this poly type");
#endif
	}

	template<class T>
	void AddType()
	{
		DEBUG_ASSERT(k_type_index<T> < k_max_offsets, "This types index is too large");

		m_type_offsets[k_type_index<T>] = m_total_size;
		m_type_constructors[k_type_index<T>] = Construct<T>;
		m_type_destructors[k_type_index<T>] = Destruct<T>;
		m_total_size += sizeof(T);
	}

	template<class T>
	const T* Get(const MainT* poly) const
	{
		DEBUG_ASSERT(k_type_index<T> < k_max_offsets, "This types index is too large");
		DEBUG_ASSERT(m_type_offsets[k_type_index<T>] != 0, "Either T == MainT or this poly doesn't have this type");

		const std::byte* ptr = reinterpret_cast<const std::byte*>(poly);

#if defined(POLY_DEBUG)
		DEBUG_ASSERT(m_created.contains(ptr), "We didn't create this poly");
#endif

		return reinterpret_cast<const T*>(ptr + m_type_offsets[k_type_index<T>]);
	}

	template<class T>
	T* Get(MainT* poly) const
	{
		return const_cast<T*>(Get<T>(const_cast<const MainT*>(poly)));
	}

	template<class T>
	const T& Get(const MainT& poly) const
	{
		return *Get<T>(&poly);
	}

	template<class T>
	T& Get(MainT& poly) const
	{
		return *Get<T>(&poly);
	}

	// Create a poly of this type
	MainT* CreatePoly()
	{
		DEBUG_ASSERT(m_total_size > 0, "We should have entries for the poly type");

		std::byte* ptr = reinterpret_cast<std::byte*>(malloc(m_total_size));

		for (size_t i = 0; i < k_max_offsets; i++)
		{
			if (m_type_constructors[i] != nullptr)
			{
				m_type_constructors[i](ptr + m_type_offsets[i]);
			}
		}

#if defined(POLY_DEBUG)
		m_created.insert(ptr);
#endif
		return reinterpret_cast<MainT*>(ptr);
	}

	// Destroy a poly of this type
	void DestroyPoly(MainT* poly)
	{
		DEBUG_ASSERT(poly != nullptr, "A valid poly should be provided for destruction");

		std::byte* ptr = reinterpret_cast<std::byte*>(poly);

#if defined(POLY_DEBUG)
		m_created.erase(ptr);
#endif

		for (size_t i = 0; i < k_max_offsets; i++)
		{
			if (m_type_destructors[i] != nullptr)
			{
				m_type_destructors[i](ptr + m_type_offsets[i]);
			}
		}

		free(ptr);
	}

	// Get the size of the whole poly type
	uint16_t GetSize() const
	{
		return m_total_size;
	}

private:
	uint16_t m_total_size = 0;
	std::array<uint16_t, k_max_offsets> m_type_offsets = { 0 };
	std::array<FactoryCB, k_max_offsets> m_type_constructors = { nullptr };
	std::array<FactoryCB, k_max_offsets> m_type_destructors = { nullptr };

#if defined(POLY_DEBUG)
	robin_hood::unordered_set<const std::byte*> m_created;
#endif
};