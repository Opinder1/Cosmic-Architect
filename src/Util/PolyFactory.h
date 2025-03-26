#pragma once

#include "Poly.h"
#include "UUID.h"

#include <robin_hood/robin_hood.h>

#include <bitset>

template<class TypeT>
class PolyPtr;

template<class ArchetypeT>
class PolyFactory
{
	template<class FactoryT>
	friend class PolyPtr;

	using Ptr = PolyPtr<PolyFactory>;
	using Header = typename ArchetypeT::Header;

public:
	using Archetype = std::bitset<ArchetypeT::k_num_types>;

private:
	struct Entry
	{
		Archetype type;
		ArchetypeT* archetype = nullptr;
		Header* header = nullptr;
	};

public:
	PolyFactory() {}

	Ptr CreatePoly(UUID id)
	{
		auto&& [it, emplaced] = m_entries.emplace(id, Entry{});

		if (emplaced)
		{
			Entry& entry = it->second;

			entry.archetype = &m_archetypes.at(entry.type);

			entry.header = entry.archetype->CreatePoly();
		}

		return Ptr(&it->second);
	}

	robin_hood::unordered_map<Archetype, ArchetypeT> m_archetypes;
	robin_hood::unordered_map<UUID, Entry> m_entries;
};

template<class FactoryT>
class PolyPtr
{
	using Entry = typename FactoryT::Entry;

public:
	PolyPtr(Entry* entry) : m_entry(entry) {}

	template<class T>
	T& Get() const
	{
		return m_entry->archetype->Get<T>(*m_entry->header);
	}

	template<class T>
	T* TryGet() const
	{
		return m_entry->archetype->Get<T>(m_entry->header);
	}

	template<class T>
	bool Has() const
	{
		return m_entry->archetype->Get<T>(m_entry->header) != nullptr;
	}

	template<auto Member,
		class Ret = get_member_type<decltype(Member)>::type,
		class Class = get_member_class<decltype(Member)>::type>
	Ret& Var() const
	{
		return m_entry->archetype->Get<Class>(m_entry->header)->*Member;
	}

	template<class T, class Ret>
	Ret& operator->*(Ret T::* Member) const
	{
		return m_entry->archetype->Get<T>(m_entry->header)->*Member;
	}

	operator bool() const
	{
		return m_entry != nullptr;
	}

private:
	Entry* m_entry;
};