#pragma once

#include "SpatialCoord.h"

namespace voxel_game
{
	class SpatialAABB
	{
	public:
		SpatialAABB();

	private:
		SpatialCoord3D position;
		godot::Vector3i size;
	};
}