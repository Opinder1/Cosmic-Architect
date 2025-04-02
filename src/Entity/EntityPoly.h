#pragma once

#include "Util/PolyFactory.h"

namespace voxel_game::entity
{
	struct Type : PolyType<Type, 128>
	{
		using PolyType::PolyType;
	};

	using Factory = PolyFactory<Type>;

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