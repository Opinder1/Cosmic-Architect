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

	class Type : public PolyType<Type, 128>
	{
	public:
		using EventCallback = cb::Callback<void(Simulation&, Ptr)>;

		using EventCallbacks = std::vector<EventCallback>;
		using TypeCallbacks = std::array<EventCallbacks, to_underlying(Event::Count)>;

		Type();

		void InitType(PolyArchetypeRegistry<Type>& factory, ID id);

		void AddCallback(Event event, EventCallback callback);

		void DoEvent(Simulation& simulation, Ptr poly, Event event) const;

	private:
		TypeCallbacks m_type_callbacks;
	};

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
		robin_hood::unordered_map<TypeID, CallbackEntries> m_callbacks;
	};

	using Ptr = Type::Ptr;
	using WRef = Factory::WeakRef;
	using Ref = Factory::Ref;
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