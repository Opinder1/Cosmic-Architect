#include "SpatialModule.h"
#include "SpatialComponents.h"
#include "SpatialProcessors.h"

#include "Simulation/SimulationComponents.h"

#include "Util/Debug.h"
#include "Util/Callback.h"

#include <flecs/flecs.h>

namespace voxel_game
{
	// System to keep alive all nodes around a loader and request the loading of any missing
	void LoaderKeepAliveNodes(
		SpatialWorld3DComponent& spatial_world,
		const SpatialLoader3DComponent& spatial_loader,
		SpatialCommands3DComponent& spatial_commands,
		const SimulationGlobal& world_time)
	{
		PARALLEL_ACCESS(spatial_world, world_time);

		SpatialLoader3DNodeProcessor(spatial_world, spatial_loader, [&](SpatialCoord3D coord, SpatialNode3D* node)
		{
			if (node != nullptr)
			{
				node->last_update_time = world_time.frame_start;
			}
			else
			{
				spatial_commands.scales[coord.scale].nodes_load.push_back(coord.pos);
			}
		});
	}

	// System to erase any nodes that are no longer being observed by any loader
	void SpatialWorldUnloadUnusedNodes(
		SpatialWorld3DComponent& spatial_world,
		const SpatialScale3DComponent& spatial_world_scale,
		SpatialCommands3DComponent& spatial_commands,
		const SimulationGlobal& world_time)
	{
		PARALLEL_ACCESS(spatial_world, world_time);

		SpatialWorld3DNodeProcessor(spatial_world, [&](SpatialNode3D* node)
		{
			if (world_time.frame_start - node->last_update_time > 20s)
			{
				spatial_commands.scales[spatial_world_scale.scale].nodes_unload.push_back(node->coord.pos);
			}
		});
	}

	SpatialModule::SpatialModule(flecs::world& world)
	{
		world.module<SpatialModule>("SpatialModule");

		world.import<SpatialComponents>();
		world.import<SimulationComponents>();

		// Syncs

		world.system("WorldLoaderProgressSync")
			.no_readonly()
			.kind<WorldLoaderProgressPhase>()
			.iter([](flecs::iter& it) {});

		world.system("WorldNodeProgressSync")
			.no_readonly()
			.kind<WorldNodeProgressPhase>()
			.iter([](flecs::iter& it) {});

		world.system("WorldRegionProgressSync")
			.no_readonly()
			.kind<WorldRegionProgressPhase>()
			.iter([](flecs::iter& it) {});

		world.system("WorldScaleProgressSync")
			.no_readonly()
			.kind<WorldScaleProgressPhase>()
			.iter([](flecs::iter& it) {});

		world.system("WorldProgressSync")
			.no_readonly()
			.kind<WorldProgressPhase>()
			.iter([](flecs::iter& it) {});

		// Systems

		world.system<SpatialWorld3DComponent, const SpatialLoader3DComponent, SpatialCommands3DComponent, const SimulationGlobal>("LoaderKeepAliveNodes")
			.multi_threaded()
			.kind<WorldProgressPhase>()
			.term<ParallelWorkerComponent>()
			.term_at(1).parent()
			.term_at(4).src<SimulationGlobal>()
			.each(LoaderKeepAliveNodes);

		world.system<SpatialWorld3DComponent, const SpatialScale3DComponent, SpatialCommands3DComponent, const SimulationGlobal>("SpatialWorldUnloadUnusedNodes")
			.multi_threaded()
			.kind<WorldProgressPhase>()
			.term<ParallelWorkerComponent>()
			.term_at(1).parent()
			.term_at(4).src<SimulationGlobal>()
			.each(SpatialWorldUnloadUnusedNodes);
	}

	SpatialNode3D* SpatialModule::GetNode(const SpatialWorld3D& world, SpatialCoord3D coord)
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