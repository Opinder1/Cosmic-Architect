#pragma once

#include <flecs/flecs.h>

namespace voxel_game::voxelrender
{
	struct Module
	{
		Module(flecs::world& world);
	};
}