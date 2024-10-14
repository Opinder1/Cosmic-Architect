#pragma once

#include "SpatialComponents.h"

namespace voxel_game::spatial3d
{
	// Run a callback for a node and all of its children
	template<class Callable>
	void ForEachChildNodeRecursive(Node* node, Callable&& callback)
	{
		if (node->children_mask == 0)
		{
			return;
		}

		for (Node* node : node->children_array)
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
		for (; start.y < end.y; start.y++)
		for (; start.z < end.z; start.z++)
		{
			callback(start);
		}
	}

	// Run a callback for all the nodes in a sphere
	template<class Callable>
	void ForEachCoordInSphere(godot::Vector3 pos, double radius, Callable&& callback)
	{
		const godot::Vector3i start = pos - godot::Vector3i(radius, radius, radius);
		const godot::Vector3i end = pos + godot::Vector3i(radius, radius, radius);

		// This shifts the grid by 0.5 so that we use the center of each node instead of the corner
		pos -= godot::Vector3(0.5, 0.5, 0.5);

		const double radius_squared = radius * radius;

		godot::Vector3i it;

		for (it.x = start.x; it.x < end.x; it.x++)
		for (it.y = start.y; it.y < end.y; it.y++)
		for (it.z = start.z; it.z < end.z; it.z++)
		{
			if (pos.distance_squared_to(it) < radius_squared)
			{
				callback(it);
			}
		}
	}
}