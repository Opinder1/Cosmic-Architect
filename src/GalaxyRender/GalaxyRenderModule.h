#pragma once

#include <flecs/flecs.h>

namespace voxel_game::galaxyrender
{
	struct Module
	{
		Module(flecs::world& world);
	};
}