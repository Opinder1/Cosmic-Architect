#include "SpatialComponents.h"

namespace voxel_game::spatial3d
{
	Scale& GetScale(World& spatial_world, Poly scale_poly)
	{
		return scale_poly.GetEntry<Scale>(spatial_world.scale_entry);
	}

	const Scale& GetScale(const World& spatial_world, Poly scale_poly)
	{
		return scale_poly.GetEntry<Scale>(spatial_world.scale_entry);
	}

	Scale& GetScale(World& spatial_world, uint8_t scale_index)
	{
		return spatial_world.scales[scale_index].GetEntry<Scale>(spatial_world.scale_entry);
	}

	const Scale& GetScale(const World& spatial_world, uint8_t scale_index)
	{
		return spatial_world.scales[scale_index].GetEntry<Scale>(spatial_world.scale_entry);
	}

	Node& GetNode(World& spatial_world, Poly node_poly)
	{
		return node_poly.GetEntry<Node>(spatial_world.node_entry);
	}

	const Node& GetNode(const World& spatial_world, Poly node_poly)
	{
		return node_poly.GetEntry<Node>(spatial_world.node_entry);
	}

	Poly GetNode(const World& world, Coord coord)
	{
		DEBUG_ASSERT(coord.scale < k_max_world_scale, "The coordinates scale is out of range");

		const Scale& scale = GetScale(world, coord.scale);

		NodeMap::const_iterator it = scale.nodes.find(coord.pos);

		if (it == scale.nodes.end())
		{
			return Poly{};
		}

		return it->second;
	}
}