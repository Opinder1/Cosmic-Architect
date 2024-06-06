#pragma once

#include "Spatial/SpatialComponents.h"

#include <godot_cpp/variant/vector3i.hpp>

#include <flecs/flecs.h>

namespace voxel_game
{
	struct UniverseNode : SpatialNode3D
	{
		std::vector<flecs::entity_t> entities;

		std::vector<godot::Vector3i> galaxies_to_load;
	};

	struct UniverseScale : SpatialScale3D
	{

	};

	struct UniverseComponent {};

	struct UniverseObjectComponent {};

	struct UniverseComponents
	{
		UniverseComponents(flecs::world& world);
	};
}