#pragma once

#include "SpatialComponents.h"
#include "SpatialTraverse.h"

#include "Util/Callback.h"
#include "Util/Debug.h"

#define PARALLEL_ACCESS(...) __VA_ARGS__;

namespace voxel_game
{
	// Wrapper to run world loaders in different threads
	template<class Callable>
	void SpatialLoader3DNodeProcessor(SpatialWorld3DComponent& spatial_world, const SpatialLoader3DComponent& spatial_loader, Callable&& processor)
	{
		PARALLEL_ACCESS(spatial_world);

		for (uint8_t scale_index = spatial_loader.min_lod; scale_index < spatial_loader.max_lod; scale_index++)
		{
			const SpatialScale3D& spatial_scale = spatial_world.scales[scale_index];

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
	void SpatialNode3DNodeProcessor(SpatialWorld3DComponent& spatial_world, const SpatialNode3DWorkerComponent& node_worker, Callable&& processor)
	{
		PARALLEL_ACCESS(spatial_world);

		SpatialScale3D& scale = spatial_world.scales[node_worker.node.scale];

		auto it = scale.nodes.find(node_worker.node.pos);

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
	void SpatialRegion3DNodeProcessor(SpatialWorld3DComponent& spatial_world, const SpatialRegion3DWorkerComponent& region_worker, Callable&& processor)
	{
		PARALLEL_ACCESS(spatial_world);

		uint32_t scale_index = region_worker.region.scale;
		godot::Vector3i start = region_worker.region.pos;
		godot::Vector3i end = region_worker.region.pos + region_worker.region.size;

		SpatialScale3D& scale = spatial_world.scales[scale_index];

		ForEachCoordInRegion(start, end, [&spatial_world, &scale, &processor](godot::Vector3i pos)
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
	void SpatialScale3DNodeProcessor(SpatialWorld3DComponent& spatial_world, const SpatialScale3DWorkerComponent& scale_worker, Callable&& processor)
	{
		PARALLEL_ACCESS(spatial_world);

		SpatialScale3D& scale = spatial_world.scales[scale_worker.scale];

		for (auto&& [pos, node] : scale.nodes)
		{
			processor(node.get());
		}
	}

	// Wrapper to run worlds in different threads
	template<class Callable>
	void SpatialWorld3DNodeProcessor(SpatialWorld3DComponent& spatial_world, Callable&& processor)
	{
		for (SpatialScale3D& scale : spatial_world.scales)
		{
			for (auto&& [pos, node] : scale.nodes)
			{
				processor(node.get());
			}
		}
	}

	// System to process all commands added for each node
	template<class Processor>
	void SpatialScale3DLoaderProcessor(flecs::world& stage, SpatialWorld3DComponent& spatial_world, const SpatialScale3DWorkerComponent& scale_worker, Processor&& processor)
	{
		PARALLEL_ACCESS(spatial_world);

		uint8_t scale_index = scale_worker.scale;

		SpatialScale3D& scale = spatial_world.scales[scale_worker.scale];

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
	void SpatialScale3DLoadCommandsProcessor(flecs::world& stage, SpatialWorld3DComponent& spatial_world, const SpatialScale3DWorkerComponent& scale_worker, Processor&& processor)
	{
		PARALLEL_ACCESS(spatial_world);

		flecs::query<SpatialLoadCommands3DComponent> staged_load_commands_query(stage, spatial_world.load_commands_query);

		uint8_t scale_index = scale_worker.scale;

		SpatialScale3D& scale = spatial_world.scales[scale_index];

		// For each command list that is a child of the world
		staged_load_commands_query.each([&scale_index, &scale, &processor](SpatialLoadCommands3DComponent& load_commands)
		{
			PARALLEL_ACCESS(load_commands);

			std::vector<godot::Vector3i>& scale_commands = load_commands.scales[scale_index];

			for (godot::Vector3i& pos : scale_commands)
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
			}
		});
	}

	// System to process all commands added for each node
	template<class Processor>
	void SpatialScale3DUnloadCommandsProcessor(flecs::world& stage, SpatialWorld3DComponent& spatial_world, const SpatialScale3DWorkerComponent& scale_worker, Processor&& processor)
	{
		PARALLEL_ACCESS(spatial_world);

		flecs::query<SpatialUnloadCommands3DComponent> staged_unload_commands_query(stage, spatial_world.unload_commands_query);

		uint8_t scale_index = scale_worker.scale;

		SpatialScale3D& scale = spatial_world.scales[scale_index];

		// For each command list that is a child of the world
		staged_unload_commands_query.each([&scale_index, &scale, &processor](SpatialUnloadCommands3DComponent& unload_commands)
		{
			PARALLEL_ACCESS(unload_commands);

			std::vector<godot::Vector3i>& scale_commands = unload_commands.scales[scale_index];

			for (godot::Vector3i& pos : scale_commands)
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
			}
		});
	}
}