#pragma once

#include "EntityPoly.h"

namespace voxel_game::entity
{
	struct CName
	{
		godot::String name;
	};

	struct CParent
	{
		Ptr parent;
	};
}