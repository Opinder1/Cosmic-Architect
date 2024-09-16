#pragma once

#include "Util/Time.h"

#include <flecs/flecs.h>

namespace voxel_game::spatial3d
{
	struct Module
	{
		Module(flecs::world& world);
	};

	struct Coord;
	struct Node;
	struct World;

	// Add scale workers to a world for each scale the world currently has
	void AddScaleWorkers(flecs::entity spatial_world_entity);

	void RemoveScaleWorkers(flecs::entity spatial_world_entity);

	Node* GetNode(const World& world, Coord coord);
}