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
		Ref prev_neighbour;
		Ref next_neighbour;

		size_t children = 0;
		Ref first_child;
	};
}