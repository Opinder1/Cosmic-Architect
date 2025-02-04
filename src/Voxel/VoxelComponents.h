#pragma once

#include "Util/Poly.h"
#include "Util/TinyOctree.h"
#include "Util/SmallVector.h"

#include <flecs/flecs.h>

#include <robin_hood/robin_hood.h>

namespace voxel_game::voxel
{
	constexpr const size_t k_max_voxel_types = UINT16_MAX;

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
	struct Node
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

	struct Scale {};

	struct World
	{
		PolyEntry<Node> node_entry;
		PolyEntry<Scale> scale_entry;
	};

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

		flecs::entity_t item = 0 ; // The related item type for this voxel type
		flecs::entity_t interact = 0; // The entity that dictates our interaction outcome

		godot::Color(*color)(uint16_t) = nullptr; // Convert the block to a color when displayed in things like a minimap

		godot::RID texture;
		godot::RID custom_mesh;

		bool is_invisible : 1;
		bool is_transparent : 1;
		bool is_mesh_detached : 1;
		bool is_breakable : 1;
		bool is_hoverable : 1;

		uint16_t break_time = 0;
	};

	struct VoxelType
	{
		flecs::entity_t entity = 0; // The entity that this cache is getting from

		VoxelTypeCache cache;
	};

	struct Context
	{
		GrowingSmallVector<VoxelType, 1024> types; // A cache of data for all known voxel types to avoid accessing the related entities
	};
}