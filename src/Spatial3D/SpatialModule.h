#pragma once

#include "Util/Time.h"
#include "Util/Poly.h"

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
	void AddScaleMarkers(flecs::entity world_entity, uint8_t first, uint8_t last);

	void RemoveScaleMarkers(flecs::entity world_entity);

	Poly GetNode(const World& world, Coord coord);
}