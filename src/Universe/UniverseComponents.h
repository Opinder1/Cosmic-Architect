#pragma once

#include "Util/Poly.h"

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

	struct World
	{
		size_t main_seed = 0;
	};

	// This entitiy is a universe which has a specialised spatial world
	struct CWorld {};
}