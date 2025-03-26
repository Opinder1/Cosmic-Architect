#pragma once

#include "Debug.h"
#include "Nocopy.h"
#include "Util/Util.h"

#include <robin_hood/robin_hood.h>

#include <array>
#include <vector>
#include <memory>

#define POLY_DEBUG DEBUG_ENABLED

using PolyComponentCB = void (*)(std::byte*);

template<class T>
static void PolyComponentConstruct(std::byte* ptr)
{
	new (reinterpret_cast<T*>(ptr)) T();
}

template<class T>
static void PolyComponentDestruct(std::byte* ptr)
{
	std::destroy_at(reinterpret_cast<T*>(ptr));
}

// A system for creating runtime defined structs which are efficently allocated in memory
template<class DerivedT, size_t N>
class PolyType : Nocopy, Nomove
{
public:
	constexpr static const uint16_t k_invalid_poly_offset = UINT16_MAX;
	constexpr static const size_t k_num_types = N;

	template<class T>
	static const size_t k_type_index;

	struct Header
	{
		PolyType* archetype = nullptr;
	};

	static const std::array<PolyComponentCB, k_num_types> k_type_constructors;
	static const std::array<PolyComponentCB, k_num_types> k_type_destructors;

public:
	PolyType()
	{
		m_type_offsets.fill(k_invalid_poly_offset);

		AddType<Header>();
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
		DEBUG_ASSERT(k_type_index<T> < k_num_types, "This types index is too large");

		m_type_offsets[k_type_index<T>] = m_total_size;
		m_total_size += sizeof(T);
	}

	template<class T>
	const T* Get(const Header* poly) const
	{
		DEBUG_ASSERT(k_type_index<T> < k_num_types, "This types index is too large");
		DEBUG_ASSERT(m_type_offsets[k_type_index<T>] != 0, "Either T == HeaderT or this poly doesn't have this type");

		const std::byte* ptr = reinterpret_cast<const std::byte*>(poly);

#if defined(POLY_DEBUG)
		DEBUG_ASSERT(m_created.contains(ptr), "We didn't create this poly");
#endif

		return reinterpret_cast<const T*>(ptr + m_type_offsets[k_type_index<T>]);
	}

	template<class T>
	T* Get(Header* poly) const
	{
		return const_cast<T*>(Get<T>(const_cast<const Header*>(poly)));
	}

	template<class T>
	const T& Get(const Header& poly) const
	{
		return *Get<T>(&poly);
	}

	template<class T>
	T& Get(Header& poly) const
	{
		return *Get<T>(&poly);
	}

	// Create a poly of this type
	Header* CreatePoly()
	{
		DEBUG_ASSERT(m_total_size > 0, "We should have entries for the poly type");

		std::byte* ptr = reinterpret_cast<std::byte*>(malloc(m_total_size));

		for (size_t i = 0; i < k_num_types; i++)
		{
			if (m_type_offsets[i] != k_invalid_poly_offset)
			{
				k_type_constructors[i](ptr + m_type_offsets[i]);
			}
		}

#if defined(POLY_DEBUG)
		m_created.insert(ptr);
#endif
		Header* header = reinterpret_cast<Header*>(ptr);

		header->archetype = this;

		return header;
	}

	// Destroy a poly of this type
	void DestroyPoly(Header* poly)
	{
		DEBUG_ASSERT(poly != nullptr, "A valid poly should be provided for destruction");

		std::byte* ptr = reinterpret_cast<std::byte*>(poly);

#if defined(POLY_DEBUG)
		DEBUG_ASSERT(m_created.contains(ptr), "We didn't create this poly");
		m_created.erase(ptr);
#endif

		for (size_t i = 0; i < k_num_types; i++)
		{
			if (m_type_offsets[i] != k_invalid_poly_offset)
			{
				k_type_destructors[i](ptr + m_type_offsets[i]);
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
	std::array<uint16_t, k_num_types> m_type_offsets;
	uint16_t m_total_size = 0;

#if defined(POLY_DEBUG)
	robin_hood::unordered_set<const std::byte*> m_created;
#endif
};

template<class ArchetypeT>
class PolyRef
{
	using Header = typename ArchetypeT::Header;

public:
	PolyRef() : m_poly(nullptr) {}
	PolyRef(Header* poly) : m_poly(poly) {}

	template<class T>
	T& Get() const
	{
		return m_poly->archetype->Get<T>(*m_poly);
	}

	template<class T>
	T* TryGet() const
	{
		return m_poly->archetype->Get<T>(m_poly);
	}

	template<class T>
	bool Has() const
	{
		return m_poly->archetype->Get<T>(m_poly) != nullptr;
	}

	template<auto Member,
		class Ret = get_member_type<decltype(Member)>::type,
		class Class = get_member_class<decltype(Member)>::type>
	Ret& Var() const
	{
		return m_poly->archetype->Get<Class>(m_poly)->*Member;
	}

	template<class T, class Ret>
	Ret& operator->*(Ret T::* Member) const
	{
		return m_poly->archetype->Get<T>(m_poly)->*Member;
	}

	void Destroy()
	{
		m_poly->archetype->DestroyPoly(m_poly);
		m_poly = nullptr;
	}

	operator bool() const
	{
		return m_poly != nullptr;
	}

private:
	Header* m_poly;
};