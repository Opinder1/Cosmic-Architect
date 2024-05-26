#include "SpatialAABB.h"

namespace voxel_game
{

	SpatialAABB::SpatialAABB()
	{}

	SpatialAABB::SpatialAABB(godot::Vector3i first, godot::Vector3i second, uint32_t scale) :
		first(first),
		second(second),
		scale(scale)
	{}

	SpatialAABB::SpatialAABB(SpatialCoord3D coord, godot::Vector3i size) :
		first(coord.pos),
		second(coord.pos + size),
		scale(coord.scale)
	{}
}