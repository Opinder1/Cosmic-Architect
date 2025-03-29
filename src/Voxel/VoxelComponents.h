#pragma once

#include "VoxelWorld.h"

#include "Util/TinyOctree.h"
#include "Util/SmallVector.h"

#include <robin_hood/robin_hood.h>

namespace voxel_game::voxel
{
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