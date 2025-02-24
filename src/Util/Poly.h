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

	inline static size_t k_total_type_indexes = 0;
	template<class T>
	inline static size_t k_type_index = 0;

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
	static void RegisterType()
	{
		DEBUG_ASSERT(k_total_type_indexes != k_max_offsets, "We don't support this many types for a poly");

		k_type_index<T> = k_total_type_indexes;
		k_total_type_indexes++;
	}

	template<class T>
	void AddType()
	{
		m_type_offsets[k_type_index<T>] = m_total_size;
		m_type_constructors[k_type_index<T>] = Construct<T>;
		m_type_destructors[k_type_index<T>] = Construct<T>;
		m_total_size += sizeof(T);
	}

	template<class T>
	const T* Get(const MainT* poly) const
	{
		return reinterpret_cast<const T*>(poly + m_type_offsets[k_type_index<T>]);
	}

	template<class T>
	T* Get(MainT* poly) const
	{
		return reinterpret_cast<T*>(poly + m_type_offsets[k_type_index<T>]);
	}

	template<class T>
	const T& Get(const MainT& poly) const
	{
		return *reinterpret_cast<const T*>(&poly + m_type_offsets[k_type_index<T>]);
	}

	template<class T>
	T& Get(MainT& poly) const
	{
		return *reinterpret_cast<T*>(&poly + m_type_offsets[k_type_index<T>]);
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
		m_created.insert(reinterpret_cast<MainT*>(ptr));
#endif
		return reinterpret_cast<MainT*>(ptr);
	}

	// Destroy a poly of this type
	void DestroyPoly(MainT* poly)
	{
		DEBUG_ASSERT(poly != nullptr, "A valid poly should be provided for destruction");

#if defined(POLY_DEBUG)
		m_created.erase(poly);
#endif

		std::byte* ptr = reinterpret_cast<std::byte*>(poly);

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
	robin_hood::unordered_set<MainT*> m_created;
#endif
};