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
	struct Node : spatial3d::Node
	{
		std::vector<flecs::entity_t> entities;
	};

	// This is a specialised scale for a universe
	struct Scale : spatial3d::Scale {};

	// This entitiy is a universe which has a specialised spatial world
	struct Universe
	{
		PolyEntry<Node> node_entry;
	};
}