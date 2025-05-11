#pragma once

#include "Util/UUID.h"
#include "Util/Util.h"
#include "Util/Callback.h"
#include "Util/PolyFactory.h"

namespace voxel_game
{
	struct Simulation;
}

namespace voxel_game::entity
{
	enum class Event : uint32_t
	{
		Load, // Entity has been spawned
		Unload, // Request for entity to be cleaned up
		Update,
		TaskUpdate, // Entity being updated in its own task
		Count,
	};

	struct EventData;

	// An entity archetype that also stores all callbacks that listen for a subset of this archetypes types
	class Type : public PolyType<Type, 40>
	{
	public:
		using EventCallback = cb::Callback<void(Simulation&, EventData&)>;
		using EventCallbacks = std::vector<EventCallback>;
		using TypeCallbacks = std::array<EventCallbacks, to_underlying(Event::Count)>;

		Type();

		void InitType(PolyArchetypeRegistry<Type>& factory, ID id);

		void AddCallback(Event event, EventCallback callback);

		void DoEvent(Simulation& simulation, EventData& data, Event event) const;

	private:
		// Callbacks that are listening to types that this archetype has
		TypeCallbacks m_type_callbacks;
	};

	// A factory that creates entities and manages changing their archetype when types are added/removed
	class Factory : public PolyFactory<Type, UUID>
	{
		friend class Type;

		using EventCallback = Type::EventCallback;

		struct CallbackEntry
		{
			Event event;
			EventCallback callback;
		};

		using CallbackEntries = std::vector<CallbackEntry>;

	public:
		Factory();

		void AddCallback(TypeID types, Event event, EventCallback callback);

		template<class... Types>
		void AddCallback(Event event, EventCallback callback)
		{
			AddCallback(Type::CreateTypeID<Types...>(), event, callback);
		}

		void DoEvent(Simulation& simulation, WeakRef poly, Event event) const;

	private:
		// Callbacks that will be added to archetypes based on what types they have
		robin_hood::unordered_map<TypeID, CallbackEntries> m_callbacks;
	};

	using Ptr = Type::Ptr;
	using WRef = Factory::WeakRef;
	using Ref = Factory::Ref;

	// Data that is passed for an event. Derive from this class for event specific data
	struct EventData
	{
		WRef entity;
	};
}

namespace std
{
	template<>
	struct hash<voxel_game::entity::WRef>
	{
		size_t operator()(const voxel_game::entity::WRef& wref) const noexcept;
	};

	template<>
	struct hash<voxel_game::entity::Ref>
	{
		size_t operator()(const voxel_game::entity::Ref& ref) const noexcept;
	};
}