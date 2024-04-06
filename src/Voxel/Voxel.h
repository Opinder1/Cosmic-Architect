#pragma once

#include "Spatial/Spatial.h"
#include "Util/PackedOctree.h"

#include <flecs/flecs.h>

namespace voxel_game
{
	// A single voxel that is stored in a voxel world
	struct Block
	{
		uint16_t id = 0; // The block type which decides its behaviour
		uint16_t data = 0; // Some data that can optionally be used by the type
	};

	// A spatial node for voxels which can also be entities. This node also keeps track of the entities and factions within it.
	struct VoxelNode : SpatialNode3D
	{
		std::vector<flecs::entity_t> entities;

		PackedOctree<flecs::entity_t, 4, 0> block_entities;

		std::vector<flecs::entity_t> factions;

		bool update_children : 1;
		bool modified : 1; // This node has been modified so should be saved and loaded instead of regenerated
		bool edited : 1; // A change has been made to this node and loaders should be notified

		Block blocks[16][16][16] = {}; // This member is last since its large
	};

	struct VoxelWorld : SpatialWorld3D
	{

	};
}