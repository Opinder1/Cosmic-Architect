#pragma once

#include "Poly.h"
#include "Callback.h"

#include <robin_hood/robin_hood.h>

template<class ArchetypeT>
class PolyArchetypeRegistry
{
public:
	using Header = typename ArchetypeT::Header;
	using TypeID = typename ArchetypeT::ID;
	using Ptr = typename ArchetypeT::Ptr;

private:
	// An archetype entry that polys will reference.
	struct ArchetypeEntry : Nomove, Nocopy
	{
		ArchetypeT archetype;
		size_t refcount = 0;
		std::vector<Header*> polys;
	};

	using ArchetypeMap = robin_hood::unordered_node_map<TypeID, ArchetypeEntry>;

public:
	PolyArchetypeRegistry() {}

	// Allocate a new poly for the given archetype.
	Header* AllocatePoly(TypeID type_id)
	{
		auto&& [it, emplaced] = m_archetypes.try_emplace(type_id);

		ArchetypeEntry& entry = it->second;

		if (emplaced)
		{
			entry.archetype.InitType(*this, type_id);
		}

		entry.refcount++;

		Header* poly = entry.archetype.AllocatePoly();

		entry.polys.push_back(poly);

		return poly;
	}

	// Deallocate a poly for the given archetype.
	void DeallocatePoly(TypeID id, Header* poly)
	{
		auto it = m_archetypes.find(id);

		ArchetypeEntry& entry = it->second;

		entry.polys.erase(std::remove(entry.polys.begin(), entry.polys.end(), poly), entry.polys.end());

		entry.archetype.DeallocatePoly(poly);

		entry.refcount--;

		if (entry.refcount == 0)
		{
			m_archetypes.erase(it);
		}
	}
	
	// Change a polys type and create/destroy components as needed.
	Header* UpdatePolyType(Header* old_poly, TypeID old_type_id, TypeID new_type_id)
	{
		if (new_type_id == old_type_id)
		{
			return old_poly;
		}

		Header* new_poly = AllocatePoly(new_type_id);

		DEBUG_ASSERT(new_poly->archetype != old_poly->archetype, "The archetypes should be different");

		for (size_t i = 1; i < ArchetypeT::k_num_types; i++)
		{
			uint16_t old_type_offset = old_poly->archetype->m_type_offsets[i];
			uint16_t new_type_offset = new_poly->archetype->m_type_offsets[i];

			if (old_type_offset != ArchetypeT::k_invalid_offset)
			{
				if (new_type_offset != ArchetypeT::k_invalid_offset)
				{
					std::byte* from = (std::byte*)old_poly + old_type_offset;
					std::byte* to = (std::byte*)new_poly + new_type_offset;

					ArchetypeT::k_type_info[i].move(from, to);
				}
				else
				{
					std::byte* from = (std::byte*)old_poly + old_type_offset;

					ArchetypeT::k_type_info[i].destruct(from);
				}
			}
			else if (new_type_offset != ArchetypeT::k_invalid_offset)
			{
				std::byte* to = (std::byte*)new_poly + new_type_offset;

				ArchetypeT::k_type_info[i].construct(to);
			}
		}

		DeallocatePoly(old_type_id, old_poly);
		
		return new_poly;
	}

	template<class CallbackT>
	void TypeIterate(TypeID types, CallbackT callback)
	{
		for (auto&& [type_id, entry] : m_archetypes)
		{
			if ((type_id & types) == types)
			{
				callback(entry.archetype);
			}
		}
	}

	// Iterate over all polys that have the given components
	template<class CallbackT>
	void Iterate(TypeID types, CallbackT callback)
	{
		for (auto&& [type_id, entry] : m_archetypes)
		{
			if ((type_id & types) == types)
			{
				for (Header* poly : entry.polys)
				{
					callback(poly);
				}
			}
		}
	}

	// Iterate over all polys that have the given components. Do only part
	// of the work for the current worker.
	template<class CallbackT>
	void WorkerIterate(TypeID types, size_t worker_count, size_t worker_index, CallbackT callback)
	{
		for (auto&& [type_id, entry] : m_archetypes)
		{
			if ((type_id & types) == types)
			{
				size_t len = entry.polys.end() - entry.polys.begin();
				size_t len_per_worker = len / worker_count;

				auto begin = entry.polys.begin() + (worker_index * len_per_worker);
				auto end = entry.polys.end() + (worker_index * len_per_worker) + len_per_worker;

				for (auto it = begin; it != end; it++)
				{
					callback(*it);
				}
			}
		}
	}

private:
	ArchetypeMap m_archetypes;
};

// A factory that handles polys of multiple archetypes of the given poly type.
// It uses PolyID values to reference individual polys.
template<class ArchetypeT, class PolyID>
class PolyFactory : protected PolyArchetypeRegistry<ArchetypeT>
{
	// A poly entry that multiple refs will reference.
	struct PolyEntry
	{
		TypeID type_id;
		ArchetypeT* archetype = nullptr;
		Header* header = nullptr;
		size_t refcount = 0;
	};

	using PolyMap = robin_hood::unordered_node_map<PolyID, PolyEntry>;

	using PolyMapEntry = typename PolyMap::value_type;

public:
	// A reference to a poly that doesn't increase the refcount. Useful for thread safe per frame iteration.
	class WeakRef
	{
	public:
		WeakRef() : m_entry(nullptr) {}
		WeakRef(PolyMapEntry* entry) : m_entry(entry) {}

		PolyID GetID()
		{
			return m_entry->first;
		}

		TypeID GetTypeID()
		{
			return m_entry->second.type_id;
		}

		const ArchetypeT* GetType() const
		{
			return m_entry->second.archetype;
		}

		Ptr GetPtr() const
		{
			return Ptr{ GetHeader() };
		}

		template<class T>
		T& Get() const
		{
			return GetType()->Get<T>(*GetHeader());
		}

		template<class T>
		T* TryGet() const
		{
			return GetType()->Get<T>(GetHeader());
		}

		template<class T>
		bool Has() const
		{
			return GetTypeID().test(ArchetypeT::k_type_index<T>);
		}

		template<auto Member,
			class Ret = get_member_type<decltype(Member)>::type,
			class Class = get_member_class<decltype(Member)>::type>
		Ret& Var() const
		{
			return GetType()->Get<Class>(GetHeader())->*Member;
		}

		template<class T, class Ret>
		Ret& operator->*(Ret T::* Member) const
		{
			return GetType()->Get<T>(GetHeader())->*Member;
		}

		operator bool() const
		{
			return m_entry != nullptr;
		}

		uint64_t Hash() const
		{
			return reinterpret_cast<uint64_t>(m_entry);
		}

	private:
		Header* GetHeader() const
		{
			return m_entry->second.header;
		}

	protected:
		PolyMapEntry* m_entry;
	};

	// A reference to a poly that increases the polys refcount. Is not thread safe to create.
	class Ref : public WeakRef, Nocopy
	{
	public:
		Ref() : WeakRef() {}

		Ref(WeakRef ref) : WeakRef(ref)
		{
			if (m_entry != nullptr)
			{
				m_entry->second.refcount++;
			}
		}

		~Ref()
		{
			if (m_entry != nullptr)
			{
				m_entry->second.refcount--;
			}
		}

		Ref(Ref&& other) noexcept
		{
			std::swap(m_entry, other.m_entry);
		}

		Ref& operator=(Ref&& other) noexcept
		{
			std::swap(m_entry, other.m_entry);
			return *this;
		}

		Ref Reference()
		{
			return Ref(m_entry);
		}
	};

public:
	PolyFactory() {}

	// Get a reference to a poly with the given PolyID or create one if needed.
	Ref GetPoly(PolyID id)
	{
		auto&& [it, emplaced] = m_entries.try_emplace(id);

		PolyEntry& entry = it->second;

		if (emplaced)
		{
			entry.type_id = ArchetypeT::CreateTypeID<Header>();
			entry.header = AllocatePoly(entry.type_id);
			entry.archetype = entry.header->archetype;

			entry.archetype->ConstructPoly(entry.header);
		}

		return Ref(&*it);
	}

	// Destroy a poly.
	void DestroyPoly(PolyID id)
	{
		auto it = m_entries.find(id);

		if (it == m_entries.end())
		{
			return;
		}

		PolyEntry& entry = it->second;

		DEBUG_ASSERT(entry.refcount == 0, "This poly still has references");

		entry.archetype->DestructPoly(entry.header);

		DeallocatePoly(entry.type_id, entry.header);

		m_entries.erase(it);
	}

	// Update the type of a poly to a new type. Any components that are in both
	// will be moved while the rest will be destroyed/newly constructed.
	void SetTypes(PolyID id, TypeID new_types)
	{
		auto it = m_entries.find(id);

		if (it == m_entries.end())
		{
			return;
		}

		PolyEntry& entry = it->second;

		Header* new_poly = UpdatePolyType(entry.header, entry.type_id, new_types);

		entry.type_id = new_type_id;
		entry.archetype = new_poly->archetype;
		entry.header = new_poly;
	}

	// Add new components if they don't already exist to a poly.
	// The existing components will be moved.
	void AddTypes(PolyID id, TypeID new_types)
	{
		auto it = m_entries.find(id);

		if (it == m_entries.end())
		{
			return;
		}

		PolyEntry& entry = it->second;

		TypeID new_type_id = entry.type_id | new_types;
		
		Header* new_poly = UpdatePolyType(entry.header, entry.type_id, new_type_id);

		entry.type_id = new_type_id;
		entry.archetype = new_poly->archetype;
		entry.header = new_poly;
	}

	// Update the type of a poly to a new type. Any components that are in both
	// will be moved while the rest will be destroyed/newly constructed.
	template<class... Types>
	void SetTypes(PolyID id)
	{
		SetTypes(id, ArchetypeT::CreateTypeID<Types...>());
	}

	// Add new components if they don't already exist to a poly.
	// The existing components will be moved.
	template<class... Types>
	void AddTypes(PolyID id)
	{
		AddTypes(id, ArchetypeT::CreateTypeID<Types...>());
	}

	// Iterate over all polys that have the given components
	template<class... Types>
	void Iterate(cb::Callback<void(Header*)> callback)
	{
		Iterate(ArchetypeT::CreateTypeID<Types...>(), callback);
	}

	// Iterate over all polys that have the given components. Do only part
	// of the work for the current worker.
	template<class... Types>
	void WorkerIterate(cb::Callback<void(Header*)> callback, size_t worker_index)
	{
		WorkerIterate(ArchetypeT::CreateTypeID<Types...>(), callback, worker_index);
	}

	// Cleanup any polys that no longer have any references
	void Cleanup()
	{
		std::vector<PolyID> erase_list;

		for (auto&& [id, entry] : m_entries)
		{
			if (entry.refcount == 0)
			{
				erase_list.push_back(id);
			}
		}

		for (PolyID id : erase_list)
		{
			DestroyPoly(id);
		}
	}

private:
	PolyMap m_entries;
};