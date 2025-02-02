#pragma once


#include <flecs/flecs.h>

namespace voxel_game::spatial3d
{
	struct WorldMarker;

	struct Module
	{
		Module(flecs::world& world);
	};

	// Add scale workers to a world for each scale the world currently has
	void InitializeWorldScales(flecs::entity world_entity, WorldMarker& spatial_world);
}