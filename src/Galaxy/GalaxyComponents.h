#pragma once

#include "Spatial3D/SpatialComponents.h"

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
	struct Node
	{
		std::vector<flecs::entity_t> stars;
	};

	// This is a specialised scale for a galaxy
	struct Scale {};

	// This entity is a galaxy
	struct World
	{
		PolyEntry<Node> node_entry;
		PolyEntry<Scale> scale_entry;
	};

	// This entity is a star
	struct Star
	{
		godot::Color color;
	};
}