#include "Spatial.h"

#include "Util/Debug.h"

namespace voxel_game
{
	SpatialWorld3D* CreateNewWorld()
	{
		SpatialWorld3D* world = new SpatialWorld3D{};
		
		return world;
	}

	void DestroyWorld(SpatialWorld3D* world)
	{
		delete world;
	}

	SpatialNode3D* GetNode(SpatialWorld3D* world, SpatialCoord3D coord)
	{
		DEBUG_ASSERT(coord.scale < k_max_world_scale, "The coordinates scale is out of range");

		auto& nodes = world->scales[coord.scale];

		auto it = nodes.find(coord.pos);

		if (it == nodes.end())
		{
			return nullptr;
		}

		return it->second;
	}
}