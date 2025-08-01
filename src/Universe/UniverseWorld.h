#pragma once

#include "Entity/EntityPoly.h"

#include <vector>

namespace voxel_game::universe
{
	const size_t k_universe_generator_seed = 0;

	// This is a specialised node for a universe
	struct Node
	{
		std::vector<entity::WRef> galaxies;
	};

	// This is a specialised scale for a universe
	struct Scale {};

	struct World
	{
		size_t main_seed = 0;
	};
}