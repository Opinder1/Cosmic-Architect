#pragma once

#include "Util/TinyOctree.h"
#include "Util/SmallVector.h"

#include "Entity/EntityPoly.h"

#include <robin_hood/robin_hood.h>

namespace voxel_game::voxel
{
	struct VoxelTypeCache
	{
		VoxelTypeCache() :
			is_invisible(false),
			is_transparent(false),
			is_mesh_detached(false),
			is_breakable(false),
			is_hoverable(false)
		{
		}

		size_t refcount = 0; // Number of chunks referencing us

		entity::Ref item; // The related item type for this voxel type
		entity::Ref interact; // The entity that dictates our interaction outcome

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

	struct CWorld {};

	struct VoxelType
	{
		size_t entity = 0; // The entity that this cache is getting from

		VoxelTypeCache cache;
	};

	struct CContext : Nocopy, Nomove
	{
		GrowingSmallVector<VoxelType, 1024> types; // A cache of data for all known voxel types to avoid accessing the related entities
	};
}