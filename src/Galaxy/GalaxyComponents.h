#pragma once

#include "Spatial/SpatialComponents.h"

#include <flecs/flecs.h>

namespace flecs
{
	struct world;
}

namespace voxel_game
{
	// This is a specialised node for a galaxy
	struct GalaxyNode : SpatialNode3D
	{
		std::vector<flecs::entity_t> entities;
	};

	// This is a specialised scale for a galaxy
	struct GalaxyScale : SpatialScale3D
	{

	};

	// This entity is a galaxy
	struct GalaxyComponent {};

	// This entity is a star
	struct StarComponent
	{
		godot::Color color;
	};

	struct GalaxyComponents
	{
		GalaxyComponents(flecs::world& world);
	};
}