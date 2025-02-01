#pragma once

#include "Spatial3D/SpatialComponents.h"

#include <godot_cpp/variant/vector3i.hpp>

#include <flecs/flecs.h>

namespace voxel_game::universe
{
	struct Components
	{
		Components(flecs::world& world);
	};

	// This is a specialised node for a universe
	struct Node
	{
		std::vector<flecs::entity_t> galaxies;
	};

	// This is a specialised scale for a universe
	struct Scale {};

	// This entitiy is a universe which has a specialised spatial world
	struct World
	{
		size_t main_seed = 0;

		PolyEntry<Node> node_entry;
		PolyEntry<Scale> scale_entry;
	};
}