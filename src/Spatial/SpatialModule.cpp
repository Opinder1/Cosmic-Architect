#include "SpatialModule.h"
#include "SpatialComponents.h"
#include "SpatialProcessors.h"

#include "Physics/PhysicsComponents.h"
#include "Simulation/SimulationComponents.h"

#include "Util/Debug.h"
#include "Util/Callback.h"

#include <flecs/flecs.h>

namespace voxel_game
{
	void SpatialLoader3DExample(SpatialWorld3DComponent& spatial_world, const SpatialLoader3DComponent& spatial_loader)
	{
		PARALLEL_ACCESS(spatial_world);

		SpatialLoader3DNodeProcessor(spatial_world, spatial_loader, [](...) {});
	}

	void SpatialNode3DExample(SpatialWorld3DComponent& spatial_world, const SpatialNode3DComponent& spatial_node)
	{
		PARALLEL_ACCESS(spatial_world);

		SpatialNode3DNodeProcessor(spatial_world, spatial_node, [](...) {});
	}

	void SpatialRegion3DExample(SpatialWorld3DComponent& spatial_world, const SpatialRegion3DComponent& spatial_region)
	{
		PARALLEL_ACCESS(spatial_world);

		SpatialRegion3DNodeProcessor(spatial_world, spatial_region, [](...) {});
	}

	void SpatialScale3DExample(SpatialWorld3DComponent& spatial_world, const SpatialScale3DComponent& spatial_scale)
	{
		PARALLEL_ACCESS(spatial_world);

		SpatialScale3DNodeProcessor(spatial_world, spatial_scale, [](...) {});
	}

	void SpatialWorld3DExample(SpatialWorld3DComponent& spatial_world)
	{
		SpatialWorld3DNodeProcessor(spatial_world, [](...) {});
	}

	struct ExampleProcessor
	{
		SpatialNode3D* CreateNode(size_t scale, godot::Vector3i pos) { return new SpatialNode3D(); }

		void DestroyNode(size_t scale, godot::Vector3i pos, SpatialNode3D* node) { delete node; }
	};

	void SpatialCommands3DExample(flecs::entity entity, SpatialWorld3DComponent& spatial_world)
	{
		SpatialCommands3DProcessor(entity, spatial_world, ExampleProcessor());
	}

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
		world.import<PhysicsComponents>();
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

		// Examples

		world.system<SpatialWorld3DComponent, const SpatialLoader3DComponent>("SpatialWorldLoaderExample")
			.multi_threaded()
			.kind<WorldLoaderProgressPhase>()
			.term<ParallelWorkerComponent>()
			.term_at(1).parent()
			.each(SpatialLoader3DExample);

		world.system<SpatialWorld3DComponent, const SpatialNode3DComponent>("SpatialWorldNodeExample")
			.multi_threaded()
			.kind<WorldNodeProgressPhase>()
			.term<ParallelWorkerComponent>()
			.term_at(1).parent()
			.each(SpatialNode3DExample);

		world.system<SpatialWorld3DComponent, const SpatialRegion3DComponent>("SpatialWorldRegionExample")
			.multi_threaded()
			.kind<WorldRegionProgressPhase>()
			.term<ParallelWorkerComponent>()
			.term_at(1).parent()
			.each(SpatialRegion3DExample);

		world.system<SpatialWorld3DComponent, const SpatialScale3DComponent>("SpatialWorldScaleExample")
			.multi_threaded()
			.kind<WorldScaleProgressPhase>()
			.term<ParallelWorkerComponent>()
			.term_at(1).parent()
			.each(SpatialScale3DExample);

		world.system<SpatialWorld3DComponent>("SpatialWorldExample")
			.multi_threaded()
			.kind<WorldProgressPhase>()
			.each(SpatialWorld3DExample);

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

		world.system<SpatialWorld3DComponent>("WorldApplyCommands")
			.multi_threaded()
			.kind<WorldProgressPhase>()
			.each(SpatialCommands3DExample);
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