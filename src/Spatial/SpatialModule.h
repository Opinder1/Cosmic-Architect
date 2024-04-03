#pragma once

#include "Util/Time.h"

namespace flecs
{
	struct world;
}

namespace voxel_game
{
	struct SpatialCoord3D;
	struct SpatialNode3D;
	struct SpatialWorld3D;

	struct WorldTime
	{
		Clock::time_point frame_start;
	};

	struct WorldProgressPhase {};
	struct WorldScaleProgressPhase {};
	struct WorldNodeProgressPhase {};
	struct WorldRegionProgressPhase {};

	struct SpatialModule
	{
		SpatialModule(flecs::world& world);

		static SpatialNode3D* GetNode(const SpatialWorld3D& world, SpatialCoord3D coord);
	};
}