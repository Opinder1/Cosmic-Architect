#pragma once

#include "Poly.h"
#include "Callback.h"

#include <robin_hood/robin_hood.h>

#include <TKRZW/tkrzw_thread_util.h>

#include <thread>

enum class PolyEvent : uint32_t
{
	BeginLoad, // Entity has started loading
	BeginUnload, // Entity has started unloading

	MainUpdate, // Entity is being updated in singlethreaded mode
	TaskUpdate, // Entity being updated in its own task

	Count,
};

// A registry that manages creating polys of multiple archetypes of the same base type.
// It also supports quick iteration of all polys that have a certain set of components.

// A factory that handles polys of multiple archetypes of the given poly type.
// It uses PolyID values to reference individual polys.
template<size_t N, class PolyID>
class PolyFactory
{
public:
	class Ref;
	class WeakRef;
	struct CallbackEntry;

	using EventCallback = cb::Callback<void(WeakRef)>;
	using EventCallbacks = std::vector<EventCallback>;
	using TypeCallbacks = std::array<EventCallbacks, to_underlying(PolyEvent::Count)>;

	// An factory archetype that also stores all callbacks that listen for a subset of this archetypes types
	class Archetype : public PolyType<Archetype, N>
	{
	public:
		Archetype() {}

		void AddCallback(PolyEvent event, EventCallback callback)
		{
			m_type_callbacks[to_underlying(event)].push_back(callback);
		}

		void DoEvent(PolyEvent event, WeakRef poly) const
		{
			for (const EventCallback& callback : m_type_callbacks[to_underlying(event)])
			{
				callback(poly);
			}
		}

	private:
		// Callbacks that are listening to types that this archetype has
		TypeCallbacks m_type_callbacks;
	};

	using Header = typename PolyType<Archetype, N>::Header;
	using TypeID = typename PolyType<Archetype, N>::ID;
	using Ptr = typename PolyType<Archetype, N>::Ptr;

private:
	struct CallbackEntry
	{
		PolyEvent event;
		EventCallback callback;
	};

	// An archetype entry that polys will reference.
	struct ArchetypeEntry : Nomove, Nocopy
	{
		Archetype archetype;
		size_t refcount = 0;
		std::vector<Header*> polys;
	};

	// Use a node map so that the entry memory is stable
	using ArchetypeMap = robin_hood::unordered_node_map<TypeID, ArchetypeEntry>;

	// A poly entry that multiple refs will reference.
	struct PolyEntry
	{
		TypeID type_id; // The archetype has the type id but this avoids double indirection for a Ref
		Archetype* archetype = nullptr;
		Header* header = nullptr;
		std::atomic_size_t refcount = 0;
	};

	// Use a node map so that the entry memory is stable
	using PolyMap = robin_hood::unordered_node_map<PolyID, PolyEntry>;

	using PolyMapEntry = typename PolyMap::value_type;

	using CallbackEntries = std::vector<CallbackEntry>;

public:
	// A reference to a poly that doesn't increase the refcount. Useful for thread safe per frame iteration.
	class WeakRef
	{
	public:
		WeakRef() : m_entry(nullptr) {}
		WeakRef(PolyMapEntry* entry) : m_entry(entry) {}

		PolyID GetID() const
		{
			return m_entry->first;
		}

		TypeID GetTypeID() const
		{
			return m_entry->second.type_id;
		}

		const Archetype* GetType() const
		{
			return m_entry->second.archetype;
		}

		Header* GetHeader() const
		{
			return m_entry->second.header;
		}

		Ptr GetPtr() const
		{
			return Ptr{ GetHeader() };
		}

		void* Data()
		{
			return reinterpret_cast<void*>(GetHeader());
		}

		template<class... Types>
		bool Has() const
		{
			TypeID partial_id = Archetype::CreateTypeID<Types...>();
			return (GetTypeID() & partial_id) == partial_id;
		}

		template<class T>
		T& Get() const
		{
			DEBUG_ASSERT(Has<T>(), "We should have this type");
			return *GetType()->Get<T>(GetHeader());
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
			return Get<Class>().*Member;
		}

		template<class T, class Ret>
		Ret& operator->*(Ret T::* Member) const
		{
			return Get<T>().*Member;
		}

		template<class T>
		size_t OffsetOf() const
		{
			return GetType()->OffsetOf<T>();
		}

		template<class T, class Class>
		size_t OffsetOf(T Class::* member) const
		{
			return GetType()->OffsetOf<T, Class>(member);
		}

		operator bool() const
		{
			return m_entry != nullptr;
		}

		bool operator==(Ptr other) const
		{
			return m_entry == other.m_entry;
		}

		uint64_t Hash() const
		{
			return reinterpret_cast<uint64_t>(m_entry);
		}

	protected:
		PolyMapEntry* m_entry = nullptr;
	};

	// A reference to a poly that increases the polys refcount. Is not thread safe to create.
	class Ref : public WeakRef, Nocopy
	{
	public:
		Ref() : WeakRef() {}

		explicit Ref(WeakRef ref) : WeakRef(ref)
		{
			if (m_entry != nullptr)
			{
				m_entry->second.refcount.fetch_add(1, std::memory_order_release);
			}
		}

		~Ref()
		{
			if (m_entry != nullptr)
			{
				m_entry->second.refcount.fetch_sub(1, std::memory_order_release);
			}
		}

		Ref(Ref&& other) noexcept
		{
			m_entry = other.m_entry;
			other.m_entry = nullptr;
		}

		Ref& operator=(Ref&& other) noexcept
		{
			if (m_entry != nullptr)
			{
				m_entry->second.refcount.fetch_sub(1, std::memory_order_release);
			}

			m_entry = other.m_entry;
			other.m_entry = nullptr;
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
		std::lock_guard lock(m_mutex);

		auto&& [it, created] = m_entries.try_emplace(id);

		PolyEntry& entry = it->second;

		if (created)
		{
			entry.type_id = Archetype::CreateTypeID<Header>();
			entry.header = AllocatePoly(entry.type_id);
			entry.archetype = entry.header->archetype;

			DEBUG_ASSERT(entry.type_id == entry.archetype->GetID(), "The archetype we got doesn't have the right type");

			entry.archetype->ConstructPoly(entry.header);
		}

		return Ref(&*it);
	}

	// Update the type of a poly to a new type. Any components that are in both
	// will be moved while the rest will be destroyed/newly constructed.
	void SetTypes(PolyID id, TypeID new_type_id)
	{
		std::lock_guard lock(m_mutex);

		auto it = m_entries.find(id);

		if (it == m_entries.end())
		{
			return;
		}

		PolyEntry& entry = it->second;

		new_type_id |= Archetype::CreateTypeID<Header>();

		Header* new_poly = UpdatePolyType(entry.header, entry.type_id, new_type_id);

		entry.type_id = new_type_id;
		entry.archetype = new_poly->archetype;
		entry.header = new_poly;
	}

	// Add new components if they don't already exist to a poly.
	// The existing components will be moved.
	void AddTypes(PolyID id, TypeID new_types)
	{
		std::lock_guard lock(m_mutex);

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

	// Iterate over all polys that have the given components
	void Iterate(TypeID types, cb::Callback<void(Ptr)> callback) const
	{
		std::shared_lock lock(m_mutex);

		for (auto&& [type_id, entry] : m_archetypes)
		{
			if ((type_id & types) == types)
			{
				for (Ptr poly : entry.polys)
				{
					callback(poly);
				}
			}
		}
	}

	// Iterate over all polys that have the given components. Do only part
	// of the work for the current worker.
	void WorkerIterate(TypeID types, size_t worker_count, size_t worker_index, cb::Callback<void(Ptr)> callback) const
	{
		std::shared_lock lock(m_mutex);

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

	// Update the type of a poly to a new type. Any components that are in both
	// will be moved while the rest will be destroyed/newly constructed.
	template<class... Types>
	void SetTypes(PolyID id)
	{
		SetTypes(id, Archetype::CreateTypeID<Types...>());
	}

	// Add new components if they don't already exist to a poly.
	// The existing components will be moved.
	template<class... Types>
	void AddTypes(PolyID id)
	{
		AddTypes(id, Archetype::CreateTypeID<Types...>());
	}

	// Iterate over all polys that have the given components
	template<class... Types>
	void Iterate(cb::Callback<void(Ptr)> callback) const
	{
		Iterate(Archetype::CreateTypeID<Types...>(), callback);
	}

	// Iterate over all polys that have the given components. Do only part
	// of the work for the current worker.
	template<class... Types>
	void WorkerIterate(size_t worker_count, size_t worker_index, cb::Callback<void(Ptr)> callback) const
	{
		WorkerIterate(Archetype::CreateTypeID<Types...>(), worker_count, worker_index, callback);
	}

	template<class... Types>
	void AddCallback(PolyEvent event, EventCallback callback)
	{
		AddCallback(Archetype::CreateTypeID<Types...>(), event, callback);
	}

	void AddCallback(TypeID types, PolyEvent event, EventCallback callback)
	{
		std::lock_guard lock(m_mutex);

		// Add to future archetypes
		m_callbacks[types].push_back({ event, callback });

		// Add to existing archetypes
		for (auto&& [type_id, entry] : m_archetypes)
		{
			if ((type_id & types) == types)
			{
				entry.archetype.AddCallback(event, callback);
			}
		}
	}

	size_t GetCount() const
	{
		std::shared_lock lock(m_mutex);

		return m_entries.size();
	}

	// Cleanup any polys that no longer have any references
	void Cleanup()
	{
		std::lock_guard lock(m_mutex);

		for (auto it = m_entries.begin(); it != m_entries.end();)
		{
			PolyEntry& entry = it->second;

			if (entry.refcount.load(std::memory_order_acquire) == 0)
			{
				entry.archetype->DestructPoly(entry.header);

				DeallocatePoly(entry.type_id, entry.header);

				it = m_entries.erase(it);
			}
			else
			{
				it++;
			}
		}
	}

	void DoEvent(PolyEvent event, WeakRef poly) const
	{
		poly.GetType()->DoEvent(event, poly);
	}

private:
	void InitType(Archetype& archetype, TypeID type_id)
	{
		for (size_t i = 1; i < type_id.size(); i++)
		{
			if (type_id.test(i))
			{
				archetype.AddType(i);
			}
		}

		for (auto&& [types, entries] : m_callbacks)
		{
			if ((type_id & types) == types) // Do we have all the types that the entry requires
			{
				for (const CallbackEntry& entry : entries)
				{
					archetype.AddCallback(entry.event, entry.callback);
				}
			}
		}
	}

	// Allocate a new poly for the given archetype.
	Header* AllocatePoly(TypeID type_id)
	{
		auto&& [it, emplaced] = m_archetypes.try_emplace(type_id);

		ArchetypeEntry& entry = it->second;

		if (emplaced)
		{
			InitType(entry.archetype, type_id);
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

		for (size_t i = 1; i < Archetype::k_num_types; i++)
		{
			uint16_t old_type_offset = old_poly->archetype->m_type_offsets[i];
			uint16_t new_type_offset = new_poly->archetype->m_type_offsets[i];

			if (old_type_offset != Archetype::k_invalid_offset)
			{
				if (new_type_offset != Archetype::k_invalid_offset)
				{
					std::byte* from = (std::byte*)old_poly + old_type_offset;
					std::byte* to = (std::byte*)new_poly + new_type_offset;

					Archetype::k_type_info[i].move(from, to);
				}
				else
				{
					std::byte* from = (std::byte*)old_poly + old_type_offset;

					Archetype::k_type_info[i].destruct(from);
				}
			}
			else if (new_type_offset != Archetype::k_invalid_offset)
			{
				std::byte* to = (std::byte*)new_poly + new_type_offset;

				Archetype::k_type_info[i].construct(to);
			}
		}

		DeallocatePoly(old_type_id, old_poly);

		return new_poly;
	}

private:
	ArchetypeMap m_archetypes;

	PolyMap m_entries;

	// Callbacks that will be added to archetypes based on what types they have
	robin_hood::unordered_map<TypeID, CallbackEntries> m_callbacks;

	mutable tkrzw::SpinSharedMutex m_mutex;
};