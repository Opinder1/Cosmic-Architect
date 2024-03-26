#pragma once

#include "Fractal.h"
#include "PackedOctree.h"

#include <flecs/flecs.h>

#include <bitset>

namespace voxel_game
{
	struct Block
	{
		uint16_t id = 0;
		uint16_t data = 0;
	};

	struct VoxelNode : public FractalNode3D
	{
		std::vector<flecs::entity_t> entities;

		PackedOctree<flecs::entity_t, 4, 0> block_entities;

		std::vector<flecs::entity_t> factions;

		bool needs_lodding : 1;
		bool modified : 1;
		bool edited : 1;

		Block blocks[16][16][16] = {};
	};

	struct VoxelRegion : FractalRegion3D
	{

	};

	struct VoxelWorld : FractalWorld3D
	{

	};
}