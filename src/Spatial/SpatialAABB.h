#pragma once

#include "SpatialCoord.h"

namespace voxel_game
{
	struct SpatialAABB
	{
		SpatialAABB();
		SpatialAABB(godot::Vector3i pos, godot::Vector3i size, uint32_t scale);
		SpatialAABB(SpatialCoord3D coord, godot::Vector3i size);

		godot::Vector3i pos;
		godot::Vector3i size;
		uint32_t scale = 0;
	};
}