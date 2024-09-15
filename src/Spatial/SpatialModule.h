#pragma once

#include "Util/Time.h"

#include <flecs/flecs.h>

namespace voxel_game::spatial
{
	struct Module
	{
		Module(flecs::world& world);
	};

	struct Coord3D;
	struct Node3D;
	struct World3D;

	// Add scale workers to a world for each scale the world currently has
	void AddScaleWorkers(flecs::entity spatial_world_entity);

	void RemoveScaleWorkers(flecs::entity spatial_world_entity);

	Node3D* GetNode(const World3D& world, Coord3D coord);
}