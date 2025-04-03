#pragma once

#include "Poly.h"
#include "UUID.h"
#include "Callback.h"

#include <robin_hood/robin_hood.h>

#include <bitset>

// A factory that handles polys of multiple archetypes of the given poly type.
// It uses UUID values to reference individual polys.
template<class ArchetypeT>
class PolyFactory
{
	using Header = typename ArchetypeT::Header;

public:
	// Use a bitset for fast type logic
	using ArchetypeID = std::bitset<ArchetypeT::k_num_types>;

private:
	// An archetype entry that polys will reference.
	struct ArchetypeEntry
	{
		ArchetypeT archetype;
		size_t refcount = 0;
		std::vector<Header*> polys;
	};

	// A poly entry that multiple refs will reference.
	struct PolyEntry
	{
		ArchetypeID type_id;
		ArchetypeT* archetype = nullptr;
		Header* header = nullptr;
		size_t refcount = 0;
	};

	using ArchetypeMap = robin_hood::unordered_map<ArchetypeID, ArchetypeEntry>;
	using PolyMap = robin_hood::unordered_map<UUID, PolyEntry>;

	using PolyMapEntry = typename PolyMap::value_type;

public:
	// A reference to a poly that doesn't increase the refcount. Useful for thread safe per frame iteration.
	class WeakRef
	{
	public:
		WeakRef() : m_entry(nullptr) {}
		WeakRef(PolyMapEntry* entry) : m_entry(entry) {}

		UUID GetID()
		{
			return m_entry->first;
		}

		ArchetypeID GetTypeID()
		{
			return m_entry->second.type_id;
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
			return GetType()->Get<T>(GetHeader()) != nullptr;
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
		ArchetypeT* GetType() const
		{
			return m_entry->second.archetype;
		}

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

	// Create a type id for the given set of types.
	template<class... Types>
	constexpr static ArchetypeID CreateTypeID()
	{
		ArchetypeID archetype;

		(archetype.set(ArchetypeT::k_type_index<Types>), ...);

		return archetype;
	}

	// Get a reference to a poly with the given UUID or create one if needed.
	Ref GetPoly(UUID id)
	{
		auto&& [it, emplaced] = m_entries.try_emplace(id);

		PolyEntry& entry = it->second;

		if (emplaced)
		{
			entry.type_id = CreateTypeID<Header>();
			entry.header = AllocatePoly(entry.type_id);
			entry.archetype = entry.header->archetype;

			entry.archetype->ConstructPoly(entry.header);
		}

		return Ref(&*it);
	}

	// Update the type of a poly to a new type. Any components that are in both
	// will be moved while the rest will be destroyed/newly constructed.
	void SetTypes(UUID id, ArchetypeID new_type_id)
	{
		auto it = m_entries.find(id);

		if (it == m_entries.end())
		{
			return;
		}

		PolyEntry& entry = it->second;

		UpdateType(entry, new_type_id);
	}

	// Add new components if they don't already exist to a poly.
	// The existing components will be moved.
	void AddTypes(UUID id, ArchetypeID partial_type_id)
	{
		auto it = m_entries.find(id);

		if (it == m_entries.end())
		{
			return;
		}

		PolyEntry& entry = it->second;

		UpdateType(entry, entry.type_id | partial_type_id);
	}

	// Iterate over all polys that have the given components
	void Iterate(ArchetypeID partial_type_id, cb::Callback<void(Header*)> callback)
	{
		for (auto&& [type_id, entry] : m_archetypes)
		{
			if ((type_id & partial_type_id) == partial_type_id)
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
	void WorkerIterate(ArchetypeID partial_type_id, cb::Callback<void(Header*)> callback, size_t worker_index)
	{

	}

	// Update the type of a poly to a new type. Any components that are in both
	// will be moved while the rest will be destroyed/newly constructed.
	template<class... Types>
	void SetTypes(UUID id)
	{
		SetTypes(id, CreateTypeID<Types...>());
	}

	// Add new components if they don't already exist to a poly.
	// The existing components will be moved.
	template<class... Types>
	void AddTypes(UUID id)
	{
		AddTypes(id, CreateTypeID<Types...>());
	}

	// Iterate over all polys that have the given components
	template<class... Types>
	void Iterate(cb::Callback<void(Header*)> callback)
	{
		Iterate(CreateTypeID<Types...>(), callback);
	}

	// Iterate over all polys that have the given components. Do only part
	// of the work for the current worker.
	template<class... Types>
	void WorkerIterate(cb::Callback<void(Header*)> callback, size_t worker_index)
	{
		WorkerIterate(CreateTypeID<Types...>(), callback, worker_index);
	}

	// Cleanup any polys that no longer have any references
	void Cleanup()
	{
		std::vector<UUID> erase_list;

		for (auto&& [id, entry] : m_entries)
		{
			if (entry.refcount == 0)
			{
				erase_list.push_back(id);
			}
		}

		for (UUID id : erase_list)
		{
			DestroyPoly(id);
		}
	}

private:
	// Destroy a poly.
	void DestroyPoly(UUID id)
	{
		auto it = m_entries.find(id);

		if (it == m_entries.end())
		{
			return;
		}

		PolyEntry& entry = it->second;

		entry.archetype->DestructPoly(entry.header);

		DeallocatePoly(entry.type_id, entry.header);

		m_entries.erase(it);
	}

	// Allocate a new poly for the given archetype.
	Header* AllocatePoly(ArchetypeID type_id)
	{
		auto&& [it, emplaced] = m_archetypes.try_emplace(type_id);

		ArchetypeEntry& entry = it->second;

		if (emplaced)
		{
			for (size_t i = 1; i < type_id.size(); i++)
			{
				if (type_id.test(i))
				{
					entry.archetype.AddType(i);
				}
			}
		}

		entry.refcount++;

		Header* poly = entry.archetype.AllocatePoly();

		entry.polys.push_back(poly);

		return poly;
	}

	// Deallocate a poly for the given archetype.
	void DeallocatePoly(ArchetypeID id, Header* poly)
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
	void UpdateType(PolyEntry& entry, ArchetypeID new_type_id)
	{
		if (new_type_id == entry.type_id)
		{
			return;
		}

		Header* new_poly = AllocatePoly(new_type_id);

		ArchetypeT& new_archetype = *new_poly->archetype;

		for (size_t i = 0; i < ArchetypeT::k_num_types; i++)
		{
			bool old_exists = entry.archetype->m_type_offsets[i] != ArchetypeT::k_invalid_offset;
			bool new_exists = new_archetype.m_type_offsets[i] != ArchetypeT::k_invalid_offset;

			if (old_exists)
			{
				if (new_exists)
				{
					std::byte* from = (std::byte*)entry.header + entry.archetype->m_type_offsets[i];
					std::byte* to = (std::byte*)new_poly + new_archetype.m_type_offsets[i];

					ArchetypeT::k_type_info[i].move(from, to);
				}
				else
				{
					std::byte* from = (std::byte*)entry.header + entry.archetype->m_type_offsets[i];

					ArchetypeT::k_type_info[i].destruct(from);
				}
			}
			else if (new_exists)
			{
				std::byte* to = (std::byte*)new_poly + new_archetype.m_type_offsets[i];

				ArchetypeT::k_type_info[i].construct(to);
			}
		}

		DeallocatePoly(entry.type_id, entry.header);

		entry.type_id = new_type_id;
		entry.archetype = &new_archetype;
		entry.header = new_poly;
	}

private:
	ArchetypeMap m_archetypes;
	PolyMap m_entries;
};