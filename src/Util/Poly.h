#pragma once

#include "Debug.h"
#include "Nocopy.h"
#include "Util/Util.h"

#include <robin_hood/robin_hood.h>

#include <array>
#include <vector>
#include <memory>

#define POLY_DEBUG DEBUG_ENABLED

template<class T>
static void ComponentConstruct(std::byte* ptr)
{
	new (reinterpret_cast<T*>(ptr)) T();
}

template<class T>
static void ComponentDestruct(std::byte* ptr)
{
	std::destroy_at(reinterpret_cast<T*>(ptr));
}

template<class T>
static void ComponentMove(std::byte* from, std::byte* to)
{
	if constexpr (std::is_move_assignable_v<T>)
	{
		*reinterpret_cast<T*>(to) = std::move(*reinterpret_cast<T*>(from));
	}
	else
	{
		DEBUG_PRINT_ERROR("Can't move type");
		DEBUG_CRASH();
	}
}

struct PolyTypeInfo
{
	using ComponentConstructCB = void (*)(std::byte*);
	using ComponentDestructCB = void (*)(std::byte*);
	using ComponentMoveCB = void (*)(std::byte*, std::byte*);

	ComponentConstructCB construct = nullptr;
	ComponentDestructCB destruct = nullptr;
	ComponentMoveCB move = nullptr;
	size_t size = 0;
};

template<class T>
constexpr PolyTypeInfo MakeTypeInfo()
{
	return { ComponentConstruct<T>, ComponentDestruct<T>, ComponentMove<T>, sizeof(T) };
}

// A system for creating runtime defined structs which are efficently allocated in memory
template<class DerivedT, size_t N>
class PolyType : Nocopy, Nomove
{
	template<class T>
	friend class PolyFactory;

private:
	constexpr static const uint16_t k_invalid_offset = UINT16_MAX;
	constexpr static const size_t k_num_types = N;

	template<class T>
	static const size_t k_type_index;

	static const std::array<PolyTypeInfo, k_num_types> k_type_info;

	struct Header
	{
		PolyType* archetype = nullptr;
	};

public:
	class Ref
	{
	public:
		Ref() : m_poly(nullptr) {}
		Ref(Header* poly) : m_poly(poly) {}

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

public:
	PolyType()
	{
		m_type_offsets.fill(k_invalid_offset);

		AddType<Header>();
	}

	~PolyType()
	{
#if defined(POLY_DEBUG)
		DEBUG_ASSERT(m_created.empty(), "We should have destroyed all instances of this poly type");
#endif
	}

	void AddType(size_t index)
	{
#if defined(POLY_DEBUG)
		DEBUG_ASSERT(m_created.size() == 0, "We can't add types once we have created polys in existence");
#endif
		DEBUG_ASSERT(index < k_num_types, "This types index is too large");
		DEBUG_ASSERT(m_type_offsets[index] == k_invalid_offset, "This type is clashing with another");

		m_type_offsets[index] = m_total_size;
		m_total_size += k_type_info[index].size;
	}

	template<class T>
	void AddType()
	{
		DEBUG_ASSERT(k_type_info[k_type_index<T>].size == sizeof(T), "The type info of this type may be incorrect");

		AddType(k_type_index<T>);
	}

	template<class T>
	const T* Get(const Header* poly) const
	{
		DEBUG_ASSERT(k_type_index<T> < k_num_types, "This types index is too large");
		DEBUG_ASSERT(m_type_offsets[k_type_index<T>] != 0, "Either T == HeaderT or this poly doesn't have this type");

#if defined(POLY_DEBUG)
		DEBUG_ASSERT(m_created.contains(poly), "We didn't create this poly");
#endif

		const std::byte* ptr = reinterpret_cast<const std::byte*>(poly);

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

	Header* AllocatePoly()
	{
		DEBUG_ASSERT(m_total_size > 0, "We should have entries for the poly type");

		Header* poly = reinterpret_cast<Header*>(malloc(m_total_size));

#if defined(POLY_DEBUG)
		m_created.insert(poly);
#endif

		poly->archetype = this;

		return reinterpret_cast<Header*>(poly);
	}

	void DeallocatePoly(Header* poly)
	{
		DEBUG_ASSERT(poly != nullptr, "A valid poly should be provided for deallocation");

#if defined(POLY_DEBUG)
		DEBUG_ASSERT(m_created.contains(poly), "We didn't create this poly");
		m_created.erase(poly);
#endif

		free(poly);
	}

	void ConstructPoly(Header* poly)
	{
		DEBUG_ASSERT(poly != nullptr, "A valid poly should be provided for construction");
#if defined(POLY_DEBUG)
		DEBUG_ASSERT(m_created.contains(poly), "We didn't create this poly");
#endif

		std::byte* ptr = reinterpret_cast<std::byte*>(poly);

		for (size_t i = 1; i < k_num_types; i++)
		{
			if (m_type_offsets[i] != k_invalid_offset)
			{
				k_type_info[i].construct(ptr + m_type_offsets[i]);
			}
		}
	}

	void DestructPoly(Header* poly)
	{
		DEBUG_ASSERT(poly != nullptr, "A valid poly should be provided for destruction");
#if defined(POLY_DEBUG)
		DEBUG_ASSERT(m_created.contains(poly), "We didn't create this poly");
#endif

		std::byte* ptr = reinterpret_cast<std::byte*>(poly);

		for (size_t i = 1; i < k_num_types; i++)
		{
			if (m_type_offsets[i] != k_invalid_offset)
			{
				k_type_info[i].destruct(ptr + m_type_offsets[i]);
			}
		}
	}

	// Create a poly of this type
	Header* CreatePoly()
	{
		Header* poly = AllocatePoly();

		ConstructPoly(poly);

		return poly;
	}

	// Destroy a poly of this type
	void DestroyPoly(Header* poly)
	{
		DestructPoly(poly);

		DeallocatePoly(poly);
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
	robin_hood::unordered_set<const Header*> m_created;
#endif
};