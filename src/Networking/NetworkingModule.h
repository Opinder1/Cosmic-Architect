#pragma once

#include <flecs/flecs.h>

namespace voxel_game::network
{
	struct Module
	{
		Module(flecs::world& world);
	};
}