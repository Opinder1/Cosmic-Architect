#pragma once

#include "Debug.h"
#include "Nocopy.h"
#include "Util.h"

#include <robin_hood/robin_hood.h>

#include <array>
#include <vector>
#include <memory>
#include <bitset>

// A type that can be described as a runtime struct. One block of memory is allocated for each poly
// but the memory is made up of multiple structs that are easily accessed

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
	friend class PolyArchetypeRegistry;

	template<class T, class T2>
	friend class PolyFactory;

private:
	constexpr static const uint16_t k_invalid_offset = UINT16_MAX;
	constexpr static const size_t k_num_types = N;

	template<class T>
	static const size_t k_type_index;

	static const std::array<PolyTypeInfo, k_num_types> k_type_info;

public:
	struct Header
	{
		DerivedT* archetype = nullptr;
	};

	// Use a bitset for fast type logic
	using ID = std::bitset<k_num_types>;

public:
	// Create a type id for the given set of types.
	template<class... Types>
	constexpr static ID CreateTypeID()
	{
		ID id;

		(id.set(k_type_index<Types>), ...);

		return id;
	}

	// A wrapper around the poly pointer
	class Ptr
	{
	public:
		Ptr() : m_poly(nullptr) {}
		Ptr(Header* poly) : m_poly(poly) {}

		template<class... Types>
		bool Has() const
		{
			return m_poly->archetype->Has<Types...>();
		}

		Header* GetHeader()
		{
			return m_poly;
		}

		void* Data()
		{
			return reinterpret_cast<void*>(m_poly);
		}

		template<class T>
		T& Get() const
		{
			DEBUG_ASSERT(Has<T>(), "We should have this type");
			return *m_poly->archetype->Get<T>(m_poly);
		}

		template<class T>
		T* TryGet() const
		{
			return Has<T>() ? &Get<T>() : nullptr;
		}

		template<auto Member,
			class Ret = get_member_type<decltype(Member)>::type,
			class Class = get_member_class<decltype(Member)>::type>
		Ret& Var() const
		{
			return Get<T>().*Member;
		}

		template<class T, class Ret>
		Ret& operator->*(Ret T::* Member) const
		{
			return Get<T>().*Member;
		}

		template<class T>
		size_t OffsetOf() const
		{
			return m_poly->archetype->OffsetOf<T>();
		}

		template<class T, class Class>
		size_t OffsetOf(T Class::* member) const
		{
			return m_poly->archetype->OffsetOf<T, Class>(member);
		}

		operator bool() const
		{
			return m_poly != nullptr;
		}

		uint64_t Hash() const
		{
			return reinterpret_cast<uint64_t>(m_poly);
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
		m_id.set(index);
		m_total_size += k_type_info[index].size;
	}

	template<class T>
	void AddType()
	{
		DEBUG_ASSERT(k_type_info[k_type_index<T>].size == sizeof(T), "The type info of this type may be incorrect");

		AddType(k_type_index<T>);
	}

	ID GetID()
	{
		return m_id;
	}

	template<class... Types>
	bool Has() const
	{
		ID partial_id = CreateTypeID<Types...>();
		return (m_id & partial_id) == partial_id;
	}

	template<class T>
	T* Get(Header* poly) const
	{
		DEBUG_ASSERT(m_type_offsets[k_type_index<T>] != 0, "Either T == HeaderT or this poly doesn't have this type");

#if defined(POLY_DEBUG)
		DEBUG_ASSERT(m_created.contains(poly), "We didn't create this poly");
#endif

		std::byte* ptr = reinterpret_cast<std::byte*>(poly);

		return reinterpret_cast<T*>(ptr + m_type_offsets[k_type_index<T>]);
	}

	template<class T>
	size_t OffsetOf() const
	{
		return m_type_offsets[k_type_index<T>];
	}

	template<class T, class Class>
	size_t OffsetOf(T Class::*member) const
	{
		return OffsetOf<Class>() + offsetof_member(member);
	}

	Header* AllocatePoly()
	{
		DEBUG_ASSERT(m_total_size > 0, "We should have entries for the poly type");

		Header* poly = reinterpret_cast<Header*>(malloc(m_total_size));

#if defined(POLY_DEBUG)
		m_created.insert(poly);
#endif

		poly->archetype = static_cast<DerivedT*>(this);

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
	Ptr CreatePoly()
	{
		Header* poly = AllocatePoly();

		ConstructPoly(poly);

		return poly;
	}

	// Destroy a poly of this type
	void DestroyPoly(Ptr poly)
	{
		DestructPoly(poly.GetHeader());

		DeallocatePoly(poly.GetHeader());
	}

	// Get the size of the whole poly type
	uint16_t GetSize() const
	{
		return m_total_size;
	}

private:
	ID m_id;
	std::array<uint16_t, k_num_types> m_type_offsets;
	uint16_t m_total_size = 0;

#if defined(POLY_DEBUG)
	// A list of all polys created to check we own polys
	robin_hood::unordered_set<const Header*> m_created;
#endif
};