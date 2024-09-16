#include "SpatialAABB.h"

namespace voxel_game::spatial3d
{
	AABB::AABB()
	{}

	AABB::AABB(godot::Vector3i first, godot::Vector3i second, uint8_t scale) :
		first(first),
		second(second),
		scale(scale)
	{}

	AABB::AABB(Coord coord, godot::Vector3i size) :
		first(coord.pos),
		second(coord.pos + size),
		scale(coord.scale)
	{}
}