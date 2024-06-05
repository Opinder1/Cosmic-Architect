#pragma once

#include "Spatial/SpatialComponents.h"

#include <flecs/flecs.h>

namespace voxel_game
{
	struct UniverseComponent {};

	struct UniverseNode : SpatialNode3D
	{
		std::vector<flecs::entity_t> entities;

		std::vector<godot::Vector3i> galaxies_to_load;
	};

	struct UniverseObjectComponent {};

	struct UniverseComponents
	{
		UniverseComponents(flecs::world& world);
	};
}