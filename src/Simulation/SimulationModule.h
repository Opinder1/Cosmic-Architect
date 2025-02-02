#pragma once

#include <flecs/flecs.h>

namespace voxel_game::sim
{
	struct Module
	{
		Module(flecs::world& world);
	};
}