#pragma once

#include <flecs/flecs.h>

namespace voxel_game::galaxy
{
	struct Prefabs
	{
		Prefabs(flecs::world& world);
	};

	struct GalaxyPrefab {};
}