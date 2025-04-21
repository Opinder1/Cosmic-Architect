#pragma once

#include "Entity/EntityPoly.h"

#include "Util/TinyOctree.h"
#include "Util/SmallVector.h"

#include <robin_hood/robin_hood.h>

namespace voxel_game::voxel
{
	constexpr const size_t k_max_voxel_types = UINT16_MAX;
	constexpr const std::nullptr_t k_null_entity = nullptr;

	// A single voxel that is stored in a voxel world
	struct Voxel
	{
		uint16_t type = 0; // The voxel type which decides its behaviour
		uint16_t data = 0; // Some data that can optionally be used by the type
	};

	// A spatial node for voxels. This node also keeps track of the entities and factions within it.
	struct Node
	{
		Node() :
			update_children(false),
			modified(false),
			edited(false)
		{}

		bool update_children : 1;
		bool modified : 1; // This node has been modified so should be saved and loaded instead of regenerated
		bool edited : 1; // A change has been made to this node and loaders should be notified

		TinyOctree<entity::Ref, 4, nullptr> voxel_entities; // Assign entities for voxels

		Voxel voxels[16][16][16] = {}; // This member is last since its large
	};

	struct Scale {};

	struct World {};
}