#pragma once

#include <flecs/flecs.h>

namespace flecs
{
	struct world;
}

namespace voxel_game::galaxy
{
	struct Prefabs
	{
		Prefabs(flecs::world& world);
	};

	struct GalaxyPrefab {};
}