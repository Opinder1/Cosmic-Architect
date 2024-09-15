#pragma once

#include "Spatial/SpatialComponents.h"

#include <flecs/flecs.h>

namespace flecs
{
	struct world;
}

namespace voxel_game::galaxy
{
	struct Components
	{
		Components(flecs::world& world);
	};

	// This is a specialised node for a galaxy
	struct Node : spatial::Node3D
	{
		std::vector<flecs::entity_t> entities;
	};

	// This is a specialised scale for a galaxy
	struct Scale : spatial::Scale3D
	{

	};

	// This entity is a galaxy
	struct Galaxy {};

	// This entity is a star
	struct Star
	{
		godot::Color color;
	};
}