#pragma once

#include "Spatial/SpatialComponents.h"

#include <godot_cpp/variant/vector3i.hpp>

#include <flecs/flecs.h>

namespace voxel_game
{
	// This is a specialised node for a universe
	struct UniverseNode : SpatialNode3D
	{
		std::vector<flecs::entity_t> entities;
	};

	// This is a specialised scale for a universe
	struct UniverseScale : SpatialScale3D
	{

	};

	// This entitiy is a universe which has a specialised spatial world
	struct UniverseComponent {};

	struct UniverseComponents
	{
		UniverseComponents(flecs::world& world);
	};
}