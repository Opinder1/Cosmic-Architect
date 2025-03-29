#pragma once

#include "Entity/EntityPoly.h"

#include "Spatial3D/SpatialWorld.h"

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

		TinyOctree<entity::Ptr, 4, nullptr> voxel_entities; // Assign entities for voxels

		Voxel voxels[16][16][16] = {}; // This member is last since its large
	};

	struct Scale {};

	struct World {};

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

		entity::Ptr item = 0; // The related item type for this voxel type
		entity::Ptr interact = 0; // The entity that dictates our interaction outcome

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
}