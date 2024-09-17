#pragma once

#include "Spatial3D/SpatialComponents.h"

#include "Util/TinyOctree.h"

#include <flecs/flecs.h>

#include <robin_hood/robin_hood.h>

namespace voxel_game::voxel
{
	struct Components
	{
		Components(flecs::world& world);
	};

	// A single voxel that is stored in a voxel world
	struct Voxel
	{
		uint16_t type = 0; // The voxel type which decides its behaviour
		uint16_t data = 0; // Some data that can optionally be used by the type
	};

	// A spatial node for voxels. This node also keeps track of the entities and factions within it.
	struct Node : spatial3d::Node
	{
		Node() :
			update_children(false),
			modified(false),
			edited(false)
		{}

		flecs::entity_t owner = 0; // The entity that owns this node (Mainly used for factions but could also be world protection)

		bool update_children : 1;
		bool modified : 1; // This node has been modified so should be saved and loaded instead of regenerated
		bool edited : 1; // A change has been made to this node and loaders should be notified

		TinyOctree<flecs::entity_t, 4, 0> voxel_entities; // Assign entities for voxels

		std::vector<flecs::entity_t> entities; // Entities that are within this node

		Voxel voxels[16][16][16] = {}; // This member is last since its large
	};

	struct Scale : spatial3d::Scale {};

	struct VoxelTypeCache
	{
		VoxelTypeCache() :
			is_invisible(false),
			is_transparent(false),
			is_mesh_detached(false),
			is_breakable(false),
			is_hoverable(false)
		{}

		size_t refcount = 0; // Number of chunks referencing us

		flecs::entity_t entity = 0; // The entity that this cache is getting from
		flecs::entity_t item = 0 ; // The related item type for this voxel type
		flecs::entity_t interact = 0; // The entity that dictates our interaction outcome

		godot::Color(*color)(uint16_t) = nullptr;

		godot::RID texture;
		godot::RID custom_mesh;

		bool is_invisible : 1;
		bool is_transparent : 1;
		bool is_mesh_detached : 1;
		bool is_breakable : 1;
		bool is_hoverable : 1;

		uint16_t break_time = 0;
	};

	struct World {};

	struct Context
	{
		robin_hood::unordered_map<uint16_t, VoxelTypeCache> type_cache; // A cache of data for all known voxel types to avoid accessing the related entities
		std::vector<uint16_t> cache_check_order; // The order to check cached voxel types for removal
	};
}