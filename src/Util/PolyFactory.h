#pragma once

#include "Poly.h"
#include "UUID.h"

#include <robin_hood/robin_hood.h>

#include <bitset>

template<class ArchetypeT>
class PolyFactory
{
	using Header = typename ArchetypeT::Header;

public:
	using TypeID = std::bitset<ArchetypeT::k_num_types>;

private:
	struct ArchetypeEntry
	{
		TypeID id;
		ArchetypeT archetype;
		size_t refcount = 0;
	};

	struct PolyEntry
	{
		TypeID id;
		ArchetypeT* archetype = nullptr;
		Header* header = nullptr;
	};

	using TypeMap = robin_hood::unordered_map<TypeID, ArchetypeEntry>;
	using PolyMap = robin_hood::unordered_map<UUID, PolyEntry>;

	using PolyMapEntry = typename PolyMap::value_type;

public:
	class Ptr
	{
	public:
		Ptr() : m_entry(nullptr) {}
		Ptr(PolyMapEntry* entry) : m_entry(entry) {}

		UUID GetID()
		{
			return m_entry->first;
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

	private:
		PolyMapEntry* m_entry;
	};

public:
	PolyFactory() {}

	template<class... Types>
	static TypeID CreateTypeID()
	{
		TypeID archetype;

		(archetype.set(ArchetypeT::k_type_index<Types>), ...);

		return archetype;
	}

	Ptr CreatePoly(UUID id)
	{
		auto&& [it, emplaced] = m_entries.try_emplace(id);

		PolyEntry& entry = it->second;

		if (emplaced)
		{
			entry.id = CreateTypeID<Header>();
			entry.archetype = &RefArchetype(entry.id);
			entry.header = entry.archetype->CreatePoly();
		}

		return Ptr(&*it);
	}

	void DestroyPoly(UUID id)
	{
		auto it = m_entries.find(id);

		if (it == m_entries.end())
		{
			return;
		}

		PolyEntry& entry = it->second;

		entry.archetype->DestroyPoly(entry.header);

		UnrefArchetype(entry.id);

		m_entries.erase(it);
	}

	void SetType(UUID id, TypeID new_type_id)
	{
		auto it = m_entries.find(id);

		if (it == m_entries.end())
		{
			return;
		}

		PolyEntry& entry = it->second;

		UpdateType(entry, new_type_id);
	}

	void AddTypes(UUID id, TypeID type_id_mod)
	{
		auto it = m_entries.find(id);

		if (it == m_entries.end())
		{
			return;
		}

		PolyEntry& entry = it->second;

		UpdateType(entry, type_id_mod | entry.id);
	}

	template<class... Types>
	void SetTypes(UUID id)
	{
		SetTypes(id, CreateTypeID<Types...>());
	}

	template<class... Types>
	void AddTypes(UUID id)
	{
		AddTypes(id, CreateTypeID<Types...>());
	}

private:
	ArchetypeT& RefArchetype(TypeID id)
	{
		auto&& [it, emplaced] = m_archetypes.try_emplace(id);

		ArchetypeEntry& entry = it->second;

		if (emplaced)
		{
			entry.id = id;
			for (size_t i = 1; i < entry.id.size(); i++)
			{
				if (entry.id.test(i))
				{
					entry.archetype.AddType(i);
				}
			}
		}

		entry.refcount++;

		return entry.archetype;
	}

	void UnrefArchetype(TypeID id)
	{
		auto it = m_archetypes.find(id);

		ArchetypeEntry& entry = it->second;

		entry.refcount--;

		if (entry.refcount == 0)
		{
			m_archetypes.erase(it);
		}
	}

	void UpdateType(PolyEntry& entry, TypeID new_type_id)
	{
		if (new_type_id == entry.id)
		{
			return;
		}

		ArchetypeT& new_archetype = RefArchetype(new_type_id);

		Header* new_poly = new_archetype.AllocatePoly();

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

		entry.archetype->DeallocatePoly(entry.header);

		UnrefArchetype(entry.id);

		entry.id = new_type_id;
		entry.archetype = &new_archetype;
		entry.header = new_poly;
	}

private:
	TypeMap m_archetypes;
	PolyMap m_entries;
};