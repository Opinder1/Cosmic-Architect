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

			ForEachCoordInSphere(spatial_loader.coord.pos, spatial_loader.dist_per_lod, [&spatial_scale, scale_index, &processor](godot::Vector3i pos)
			{
				auto it = spatial_scale.nodes.find(pos);

				if (it != spatial_scale.nodes.end())
				{
					processor(SpatialCoord3D{ pos, scale_index }, it->second.get());
				}
				else
				{
					processor(SpatialCoord3D{ pos, scale_index }, nullptr);
				}
			});
		}
	}

	// Wrapper to run world nodes in different threads
	template<class Callable>
	void SpatialNode3DNodeProcessor(SpatialWorld3DComponent& spatial_world, const SpatialNode3DComponent& spatial_node, Callable&& processor)
	{
		PARALLEL_ACCESS(spatial_world);

		SpatialScale3D& scale = spatial_world.world.scales[spatial_node.node.scale];

		auto it = scale.nodes.find(spatial_node.node.pos);

		if (it == scale.nodes.end())
		{
			DEBUG_PRINT_WARN("This spatial node components node is not loaded");
			return;
		}

		SpatialNode3D* root_node = it->second.get();

		ForEachChildNodeRecursive(root_node, processor);
	}

	// Wrapper to run world regions in different threads
	template<class Callable>
	void SpatialRegion3DNodeProcessor(SpatialWorld3DComponent& spatial_world, const SpatialRegion3DComponent& spatial_region, Callable&& processor)
	{
		PARALLEL_ACCESS(spatial_world);

		uint32_t scale_index = spatial_region.region.scale;
		godot::Vector3i start = spatial_region.region.pos;
		godot::Vector3i end = spatial_region.region.pos + spatial_region.region.size;

		SpatialScale3D& scale = spatial_world.world.scales[scale_index];

		ForEachCoordInRegion(start, end, [&spatial_world, &spatial_region, &scale, &processor](godot::Vector3i pos)
		{
			auto it = scale.nodes.find(pos);

			if (it == scale.nodes.end())
			{
				DEBUG_PRINT_WARN("This spatial node components node is not loaded");
				return;
			}

			SpatialNode3D* root_node = it->second.get();

			ForEachChildNodeRecursive(root_node, processor);
		});
	}

	// Wrapper to run world scales in different threads
	template<class Callable>
	void SpatialScale3DNodeProcessor(SpatialWorld3DComponent& spatial_world, const SpatialScale3DComponent& spatial_scale, Callable&& processor)
	{
		PARALLEL_ACCESS(spatial_world);

		SpatialScale3D& scale = spatial_world.world.scales[spatial_scale.scale];

		for (auto&& [pos, node] : scale.nodes)
		{
			processor(node.get());
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
				processor(node.get());
			}
		}
	}

	// System to process all commands added for each node
	template<class Processor>
	void SpatialScale3DLoaderProcessor(flecs::world& stage, SpatialWorld3DComponent& spatial_world, const SpatialScale3DComponent& spatial_scale, Processor&& processor)
	{
		PARALLEL_ACCESS(spatial_world);

		uint8_t scale_index = spatial_scale.scale;

		SpatialScale3D& scale = spatial_world.world.scales[spatial_scale.scale];

		flecs::query<SpatialLoader3DComponent> staged_loaders_query(stage, spatial_world.loaders_query);

		// For each command list that is a child of the world
		staged_loaders_query.each([&scale, scale_index, &processor](SpatialLoader3DComponent& spatial_loader)
		{
			PARALLEL_ACCESS(spatial_loader);

			ForEachCoordInSphere(spatial_loader.coord.pos, spatial_loader.dist_per_lod, [&scale, &spatial_loader, scale_index, &processor](godot::Vector3i pos)
			{
				auto it = scale.nodes.find(pos);

				if (it != scale.nodes.end())
				{
					processor(SpatialCoord3D{ pos, scale_index }, it->second.get());
				}
				else
				{
					processor(SpatialCoord3D{ pos, scale_index }, nullptr);
				}
			});
		});
	}

	// System to process all commands added for each node
	template<class Processor>
	void SpatialWorld3DCommandsProcessor(flecs::world& stage, SpatialWorld3DComponent& spatial_world, Processor&& processor)
	{
		flecs::query<SpatialCommands3DComponent> staged_commands_query(stage, spatial_world.commands_query);

		// For each command list that is a child of the world
		staged_commands_query.each([&spatial_world, &processor](SpatialCommands3DComponent& spatial_commands)
		{
			// Commands for each scale
			for (uint8_t scale_index = 0; scale_index < k_max_world_scale; scale_index++)
			{
				SpatialCommands3DComponent::Commands& scale_commands = spatial_commands.scales[scale_index];
				SpatialScale3D& scale = spatial_world.world.scales[scale_index];

				for (godot::Vector3i& pos : scale_commands.nodes_create)
				{
					auto it = scale.nodes.find(pos);

					if (it == scale.nodes.end())
					{
						scale.nodes.emplace(pos, processor.CreateNode(scale_index, pos));
					}
				}
				scale_commands.nodes_load = std::move(scale_commands.nodes_create);

				for (godot::Vector3i& pos : scale_commands.nodes_delete)
				{
					auto it = scale.nodes.find(pos);

					if (it != scale.nodes.end())
					{
						processor.DestroyNode(scale_index, pos, it->second.get());

						scale.nodes.erase(it);
					}

				}
				scale_commands.nodes_delete.clear();
			}
		});
	}
}