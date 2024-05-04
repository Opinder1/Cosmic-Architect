#pragma once

#include "Spatial/SpatialComponents.h"

#include <flecs/flecs.h>

namespace voxel_game
{
	struct UniverseNode : SpatialNode3D
	{
		std::vector<flecs::entity_t> entities;
	};

	struct UniverseComponent {};

	struct UniverseObjectComponent {};

	struct UniverseComponents
	{
		UniverseComponents(flecs::world& world);
	};
}