#pragma once

#include "Spatial.h"

namespace voxel_game
{
	// Run a callback for a node and all of its children
	template<class Callable>
	void ForEachChildNodeRecursive(SpatialNode3D* node, Callable&& callback)
	{
		if (node->children_mask == 0)
		{
			return;
		}

		for (SpatialNode3D* node : node->children_array)
		{
			if (node != nullptr)
			{
				callback(node);

				ForEachChildNodeRecursive(node, callback);
			}
		}
	}

	// Run a callback for all the nodes in a cuboid region
	template<class Callable>
	void ForEachCoordInRegion(godot::Vector3i start, godot::Vector3i end, Callable&& callback)
	{
		if (end.x < start.x) std::swap(start.x, end.x);
		if (end.y < start.y) std::swap(start.y, end.y);
		if (end.z < start.z) std::swap(start.z, end.z);

		for (; start.x < end.x; start.x++)
		{
			for (; start.y < end.x; start.y++)
			{
				for (; start.z < end.x; start.z++)
				{
					callback(start);
				}
			}
		}
	}

	// Run a callback for all the nodes in a sphere
	template<class Callable>
	void ForEachCoordInSphere(godot::Vector3 pos, double radius, Callable&& callback)
	{
		godot::Vector3i start = pos - godot::Vector3i(radius, radius, radius);
		godot::Vector3i end = pos + godot::Vector3i(radius, radius, radius);

		for (; start.x < end.x; start.x++)
		{
			for (; start.y < end.x; start.y++)
			{
				for (; start.z < end.x; start.z++)
				{
					if (pos.distance_squared_to(start) < radius)
					{
						callback(start);
					}
				}
			}
		}
	}
}