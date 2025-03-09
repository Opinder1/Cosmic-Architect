#pragma once

#include <flecs/flecs.h>

#include <vector>

namespace voxel_game::universe
{
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
}