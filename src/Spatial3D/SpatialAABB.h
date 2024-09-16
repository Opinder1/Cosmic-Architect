#pragma once

#include "SpatialCoord.h"

namespace voxel_game::spatial3d
{
	struct AABB
	{
		AABB();
		AABB(godot::Vector3i first, godot::Vector3i second, uint8_t scale);
		AABB(Coord coord, godot::Vector3i size);

		godot::Vector3i first;
		godot::Vector3i second;
		uint8_t scale = 0;
	};
}