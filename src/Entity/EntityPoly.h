#pragma once

#include "Util/UUID.h"
#include "Util/Util.h"
#include "Util/Callback.h"
#include "Util/PolyFactory.h"

namespace voxel_game::entity
{
	// A factory that creates entities and manages changing their archetype when types are added/removed
	struct Factory : public PolyFactory<40, UUID>
	{
		using PolyFactory::PolyFactory;
	};

	using TypeID = Factory::TypeID;
	using Ptr = Factory::Ptr;
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