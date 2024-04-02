#pragma once

#include "Spatial.h"

namespace flecs
{
	struct world;
	struct entity;
}

namespace voxel_game
{
	struct SpatialModule
	{
		SpatialModule(flecs::world& world);
	};
}