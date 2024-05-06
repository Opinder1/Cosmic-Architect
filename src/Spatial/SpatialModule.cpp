#include "SpatialModule.h"
#include "SpatialComponents.h"
#include "SpatialProcessors.h"

#include "Simulation/SimulationComponents.h"

#include "Util/Debug.h"
#include "Util/Callback.h"

namespace voxel_game
{
	// System to keep alive all nodes around a loader and request the loading of any missing
	void LoaderKeepAliveNodes(
		SpatialWorld3DComponent& spatial_world,
		const SpatialLoader3DComponent& spatial_loader,
		const SimulationGlobal& world_time)
	{
		PARALLEL_ACCESS(spatial_world, world_time);

		SpatialLoader3DNodeProcessor(spatial_world, spatial_loader, [&world_time](SpatialCoord3D coord, SpatialNode3D* node)
		{
			if (node != nullptr)
			{
				node->last_update_time = world_time.frame_start;
			}
		});
	}

	// System to erase any nodes that are no longer being observed by any loader
	void SpatialWorldUnloadUnusedNodes(
		SpatialWorld3DComponent& spatial_world,
		const SpatialScale3DWorkerComponent& scale_worker,
		SpatialUnloadCommands3DComponent& spatial_commands,
		const SimulationGlobal& world_time)
	{
		PARALLEL_ACCESS(spatial_world, world_time);

		SpatialScale3DNodeProcessor(spatial_world, scale_worker, [&](SpatialNode3D* node)
		{
			if (world_time.frame_start - node->last_update_time > 20s)
			{
				spatial_commands.scales[scale_worker.scale].push_back(node->coord.pos);
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

		world.system<SpatialWorld3DComponent, const SpatialLoader3DComponent, const SimulationGlobal>("LoaderKeepAliveNodes")
			.multi_threaded()
			.kind<WorldProgressPhase>()
			.term_at(1).parent()
			.term_at(3).src<SimulationGlobal>()
			.each(LoaderKeepAliveNodes);

		world.system<SpatialWorld3DComponent, const SpatialScale3DWorkerComponent, SpatialUnloadCommands3DComponent, const SimulationGlobal>("SpatialWorldUnloadUnusedNodes")
			.multi_threaded()
			.kind<WorldProgressPhase>()
			.term_at(1).parent()
			.term_at(4).src<SimulationGlobal>()
			.each(SpatialWorldUnloadUnusedNodes);
	}

	void SpatialModule::AddSpatialScaleWorkers(flecs::world& world, flecs::entity_t spatial_world_entity)
	{
		const SpatialWorld3DComponent* spatial_world = flecs::entity(world, spatial_world_entity).get<SpatialWorld3DComponent>();

		DEBUG_ASSERT(spatial_world != nullptr, "The entity should have a spatial world to add spatial workers");

		for (uint8_t scale_index = 0; scale_index < spatial_world->max_scale; scale_index++)
		{
			flecs::entity scale_worker_entity = world.entity()
				.child_of(spatial_world_entity)
				.add<SpatialScale3DWorkerComponent>()
				.add<SpatialLoadCommands3DComponent>()
				.add<SpatialUnloadCommands3DComponent>();

			scale_worker_entity.set([scale_index](SpatialScale3DWorkerComponent& scale_worker)
			{
				scale_worker.scale = scale_index;
			});
		}
	}

	SpatialNode3D* SpatialModule::GetNode(const SpatialWorld3DComponent& world, SpatialCoord3D coord)
	{
		DEBUG_ASSERT(coord.scale < k_max_world_scale, "The coordinates scale is out of range");

		auto& nodes = world.scales[coord.scale].nodes;

		auto it = nodes.find(coord.pos);

		if (it == nodes.end())
		{
			return nullptr;
		}

		return it->second.get();
	}
}