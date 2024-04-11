#pragma once

#include "Voxel.h"

namespace flecs
{
	struct world;
}

namespace voxel_game
{
	struct VoxelComponents
	{
		VoxelComponents(flecs::world& world);
	};
}