#pragma once

#include "Spatial.h"
#include "SpatialComponents.h"
#include "SpatialTraverse.h"

#include "Util/Callback.h"
#include "Util/Debug.h"

namespace voxel_game
{
	// Wrapper to run world loaders in different threads
	template<auto callback, class... Args>
	void Loader3DParallelSystem(SpatialWorld3DComponent& spatial_world, const SpatialLoader3DComponent& spatial_loader, Args... args)
	{
		for (uint8_t scale_index = spatial_loader.min_lod; scale_index < spatial_loader.max_lod; scale_index++)
		{
			const SpatialScale3D& spatial_scale = spatial_world.world.scales[scale_index];

			ForEachCoordInSphere(spatial_loader.coord.pos, spatial_loader.dist_per_lod, [&, scale_index](godot::Vector3i pos)
			{
				auto it = spatial_scale.nodes.find(pos);

				if (it != spatial_scale.nodes.end())
				{
					callback(SpatialCoord3D{ it->first, scale_index }, it->second, spatial_world, spatial_loader, args...);
				}
				else
				{
					callback(SpatialCoord3D{}, nullptr, spatial_world, spatial_loader, args...);
				}
			});
		}
	}

	template<auto callback, class... Args>
	void Region3DParallelSystem(SpatialWorld3DComponent& spatial_world, const SpatialRegion3DComponent& spatial_world_region, Args... args)
	{
		uint32_t scale_index = spatial_world_region.region.scale;
		godot::Vector3i start = spatial_world_region.region.pos;
		godot::Vector3i end = spatial_world_region.region.pos + spatial_world_region.region.size;

		SpatialScale3D& scale = spatial_world.world.scales[scale_index];

		ForEachCoordInRegion(start, end, [&](godot::Vector3i pos)
		{
			SpatialNode3D* root_node = scale.nodes[pos];

			ForEachChildNodeRecursive(root_node, [&](SpatialNode3D* node)
			{
				callback(node, spatial_world, spatial_world_region, args...);
			});
		});
	}

	template<auto callback, class... Args>
	void Node3DParallelSystem(SpatialWorld3DComponent& spatial_world, const SpatialNode3DComponent& spatial_world_node, Args... args)
	{
		SpatialScale3D& scale = spatial_world.world.scales[spatial_world_node.node.scale];

		auto it = scale.nodes.find(spatial_world_node.node.pos);

		if (it == scale.nodes.end())
		{
			DEBUG_PRINT_WARN("This spatial node components node is not loaded");
			return;
		}

		SpatialNode3D* node = it->second;

		ForEachChildNodeRecursive(node, [&](SpatialNode3D* node)
		{
			callback(node, spatial_world, spatial_world_node, args...);
		});
	}

	template<auto callback, class... Args>
	void Scale3DParallelSystem(SpatialWorld3DComponent& spatial_world, const SpatialScale3DComponent& spatial_world_scale, Args... args)
	{
		SpatialScale3D& scale = spatial_world.world.scales[spatial_world_scale.scale];

		for (auto&& [pos, node] : scale.nodes)
		{
			callback(node, spatial_world, spatial_world_scale, args...);
		}
	}

	template<auto callback, class... Args>
	void World3DParallelSystem(SpatialWorld3DComponent& spatial_world, Args... args)
	{
		for (SpatialScale3D& scale : spatial_world.world.scales)
		{
			for (auto&& [pos, node] : scale.nodes)
			{
				callback(node, spatial_world, args...);
			}
		}
	}
}