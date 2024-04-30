#pragma once

#include "Spatial.h"
#include "SpatialComponents.h"
#include "SpatialTraverse.h"

#include "Util/Callback.h"
#include "Util/Debug.h"

#define PARALLEL_ACCESS(...) __VA_ARGS__

namespace voxel_game
{
	// Wrapper to run world loaders in different threads
	template<class Callable>
	void SpatialLoader3DNodeProcessor(SpatialWorld3DComponent& spatial_world, const SpatialLoader3DComponent& spatial_loader, Callable&& processor)
	{
		PARALLEL_ACCESS(spatial_world);

		for (uint8_t scale_index = spatial_loader.min_lod; scale_index < spatial_loader.max_lod; scale_index++)
		{
			const SpatialScale3D& spatial_scale = spatial_world.world.scales[scale_index];

			ForEachCoordInSphere(spatial_loader.coord.pos, spatial_loader.dist_per_lod, [&, scale_index](godot::Vector3i pos)
			{
				auto it = spatial_scale.nodes.find(pos);

				if (it != spatial_scale.nodes.end())
				{
					processor(SpatialCoord3D{ it->first, scale_index }, it->second);
				}
				else
				{
					processor(SpatialCoord3D{}, nullptr);
				}
			});
		}
	}

	// Wrapper to run world nodes in different threads
	template<class Callable>
	void SpatialNode3DNodeProcessor(SpatialWorld3DComponent& spatial_world, const SpatialNode3DComponent& spatial_world_node, Callable&& processor)
	{
		PARALLEL_ACCESS(spatial_world);

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
			processor(node);
		});
	}

	// Wrapper to run world regions in different threads
	template<class Callable>
	void SpatialRegion3DNodeProcessor(SpatialWorld3DComponent& spatial_world, const SpatialRegion3DComponent& spatial_world_region, Callable&& processor)
	{
		PARALLEL_ACCESS(spatial_world);

		uint32_t scale_index = spatial_world_region.region.scale;
		godot::Vector3i start = spatial_world_region.region.pos;
		godot::Vector3i end = spatial_world_region.region.pos + spatial_world_region.region.size;

		SpatialScale3D& scale = spatial_world.world.scales[scale_index];

		ForEachCoordInRegion(start, end, [&](godot::Vector3i pos)
		{
			SpatialNode3D* root_node = scale.nodes[pos];

			ForEachChildNodeRecursive(root_node, [&](SpatialNode3D* node)
			{
				processor(node);
			});
		});
	}

	// Wrapper to run world scales in different threads
	template<class Callable>
	void SpatialScale3DNodeProcessor(SpatialWorld3DComponent& spatial_world, const SpatialScale3DComponent& spatial_world_scale, Callable&& processor)
	{
		PARALLEL_ACCESS(spatial_world);

		SpatialScale3D& scale = spatial_world.world.scales[spatial_world_scale.scale];

		for (auto&& [pos, node] : scale.nodes)
		{
			processor(node);
		}
	}

	// Wrapper to run worlds in different threads
	template<class Callable>
	void SpatialWorld3DNodeProcessor(SpatialWorld3DComponent& spatial_world, Callable&& processor)
	{
		for (SpatialScale3D& scale : spatial_world.world.scales)
		{
			for (auto&& [pos, node] : scale.nodes)
			{
				processor(node);
			}
		}
	}

	// System to process all commands added for each node
	template<class Processor>
	void SpatialCommands3DProcessor(flecs::entity entity, SpatialWorld3DComponent& spatial_world, Processor&& processor)
	{
		flecs::query<SpatialCommands3DComponent> staged_commands_query(entity.world(), spatial_world.commands_query);

		// For each command list that is a child of the world
		staged_commands_query.each([&spatial_world, &processor](SpatialCommands3DComponent& spatial_commands)
		{
			// Commands for each scale
			for (size_t scale_index = 0; scale_index < k_max_world_scale; scale_index++)
			{
				SpatialCommands3DComponent::Commands& scale_commands = spatial_commands.scales[scale_index];
				SpatialScale3D& scale = spatial_world.world.scales[scale_index];

				for (godot::Vector3i& pos : scale_commands.nodes_load)
				{
					SpatialNode3D* node = processor.CreateNode(scale_index, pos);

					if (node == nullptr)
					{
						continue;
					}

					scale.nodes.try_emplace(pos, node);
				}

				scale_commands.nodes_load.clear();

				for (godot::Vector3i& pos : scale_commands.nodes_unload)
				{
					auto it = scale.nodes.find(pos);

					if (it == scale.nodes.end())
					{
						continue;
					}

					processor.DestroyNode(scale_index, pos, it->second);

					scale.nodes.erase(it);
				}

				scale_commands.nodes_unload.clear();
			}
		});
	}
}