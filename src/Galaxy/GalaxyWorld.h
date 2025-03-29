#pragma once

#include "Entity/EntityPoly.h"

#include <vector>

namespace voxel_game::galaxy
{
	// This is a specialised node for a galaxy
	struct Node
	{
		std::vector<entity::Ptr> stars;
	};

	// This is a specialised scale for a galaxy
	struct Scale {};

	// This is a specialised world for a galaxy
	struct World {};
}