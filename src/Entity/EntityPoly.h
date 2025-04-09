#pragma once

#include "Util/Util.h"
#include "Util/Callback.h"
#include "Util/PolyFactory.h"

namespace voxel_game
{
	struct Simulation;
}

namespace voxel_game::entity
{
	struct Type : PolyType<Type, 128>
	{
		using PolyType::PolyType;
	};

	using Factory = PolyFactory<Type>;

	using WRef = Factory::WeakRef;
	using Ref = Factory::Ref;

	enum class Events
	{
		Update,
		Create,
		Destroy,
		LoadStep,
		UnloadStep,
		Count,
	};

	using TypeCallback = cb::Callback<void(Simulation&, WRef)>;
	using TypeCallbacks = std::vector<TypeCallback>;

	using EventCallback = robin_hood::unordered_map<entity::Factory::ArchetypeID, TypeCallbacks>;
	using EventCallbacks = std::array<EventCallback, to_underlying(Events::Count)>;
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