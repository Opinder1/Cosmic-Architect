#pragma once

#include "Spatial/Spatial.h"

namespace voxel_game
{
	struct UniverseNode : SpatialNode3D
	{
		std::vector<flecs::entity_t> entities;
	};
}