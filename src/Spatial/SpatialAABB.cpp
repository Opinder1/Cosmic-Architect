#include "SpatialAABB.h"

namespace voxel_game
{

	SpatialAABB::SpatialAABB()
	{}

	SpatialAABB::SpatialAABB(godot::Vector3i pos, godot::Vector3i size, uint32_t scale) :
		pos(pos),
		size(size),
		scale(scale)
	{}

	SpatialAABB::SpatialAABB(SpatialCoord3D coord, godot::Vector3i size) :
		pos(coord.pos),
		size(size),
		scale(coord.scale)
	{}
}