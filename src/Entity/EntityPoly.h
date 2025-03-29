#pragma once

#include "Util/PolyFactory.h"

namespace voxel_game::entity
{
	struct Type : PolyType<Type, 128>
	{
		using PolyType::PolyType;
	};

	using Factory = PolyFactory<Type>;

	using Ptr = Factory::Ptr;
}