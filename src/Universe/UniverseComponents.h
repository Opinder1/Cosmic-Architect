#pragma once

#include "UniverseWorld.h"

#include <flecs/flecs.h>

namespace voxel_game::universe
{
	struct Components
	{
		Components(flecs::world& world);
	};

	// This entitiy is a universe which has a specialised spatial world
	struct CWorld {};
}