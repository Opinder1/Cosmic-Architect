#include "Spatial.h"

#include "Util/Debug.h"

namespace voxel_game
{
	SpatialNode3D* GetNode(const SpatialWorld3DComponent& world, SpatialCoord3D coord)
	{
		DEBUG_ASSERT(coord.scale < k_max_world_scale, "The coordinates scale is out of range");

		auto& nodes = world.scales[coord.scale].nodes;

		auto it = nodes.find(coord.pos);

		if (it == nodes.end())
		{
			return nullptr;
		}

		return it->second;
	}
}