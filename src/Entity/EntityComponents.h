#pragma once

#include "EntityPoly.h"

namespace voxel_game::entity
{
	struct CName
	{
		godot::String name;
	};

	struct CRelationship
	{
		Ref parent;
		WRef prev_neighbour;
		WRef next_neighbour;

		size_t children = 0;
		WRef first_child;
	};
}