#pragma once

#include <flecs/flecs.h>

namespace voxel_game::physics3d
{
	struct Module
	{
		Module(flecs::world& world);
	};
}