#pragma once

#include "Util/Util.h"
#include "Util/Poly.h"

#include <flecs/flecs.h>

namespace voxel_game::spatial3d
{
	struct Module
	{
		Module(flecs::world& world);
	};

	struct WorldMarker;

	// Add scale workers to a world for each scale the world currently has
	void InitializeWorldScales(flecs::entity world_entity, WorldMarker& spatial_world);
}