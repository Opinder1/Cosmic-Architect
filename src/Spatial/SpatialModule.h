#pragma once

#include "Util/Time.h"

namespace flecs
{
	struct entity;
	struct world;
}

namespace voxel_game
{
	struct SpatialCoord3D;
	struct SpatialNode3D;
	struct SpatialWorld3D;

	struct SpatialWorld3DComponent;

	struct SpatialModule
	{
		SpatialModule(flecs::world& world);

		static SpatialNode3D* GetNode(const SpatialWorld3D& world, SpatialCoord3D coord);
	};
}