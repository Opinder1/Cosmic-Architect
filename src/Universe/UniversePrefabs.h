#pragma once

#include <flecs/flecs.h>

namespace voxel_game::universe
{
	struct Prefabs
	{
		Prefabs(flecs::world& world);
	};

	struct PSimulatedUniverse {};
}