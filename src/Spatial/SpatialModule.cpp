#include "SpatialModule.h"
#include "SpatialComponents.h"
#include "SpatialParallelSystems.h"

#include "Physics/PhysicsComponents.h"
#include "Simulation/SimulationComponents.h"

#include "Util/Debug.h"
#include "Util/Callback.h"

#include <flecs/flecs.h>

namespace voxel_game
{
	// System to keep alive all nodes around a loader and request the loading of any missing
	void SpatialWorldLoaderUpdateNodes(
		SpatialCoord3D coord,
		SpatialNode3D* node,
		SpatialWorld3DComponent& spatial_world,
		const SpatialLoader3DComponent& spatial_loader,
		SpatialCommands3DComponent& spatial_commands,
		const SimulationGlobal& world_time)
	{
		if (node != nullptr)
		{
			node->last_update_time = world_time.frame_start;
		}
		else
		{
			spatial_commands.scales[coord.scale].nodes_load.push_back(coord.pos);
		}
	}

	// System to erase any nodes that are no longer being observed by any loader
	void SpatialWorldUnloadUnusedNodes(
		SpatialNode3D* node,
		SpatialWorld3DComponent& spatial_world,
		const SpatialScale3DComponent& spatial_world_scale,
		SpatialCommands3DComponent& spatial_commands,
		const SimulationGlobal& world_time)
	{
		if (world_time.frame_start - node->last_update_time > 20s)
		{
			spatial_commands.scales[spatial_world_scale.scale].nodes_unload.push_back(node->coord.pos);
		}
	}

	// System to process all commands added for each node
	void SpatialWorldApplyCommands(flecs::entity entity, SpatialWorld3DComponent& spatial_world)
	{
		auto stage_command_query = flecs::query<SpatialCommands3DComponent>(entity.world(), spatial_world.commands_query);

		// For each command list that is a child of the world
		stage_command_query.each([&spatial_world](SpatialCommands3DComponent& spatial_commands)
		{
			// Commands for each scale
			for (size_t scale_index = 0; scale_index < k_max_world_scale; scale_index++)
			{
				SpatialScaleCommands& scale_commands = spatial_commands.scales[scale_index];
				SpatialScale3D& scale = spatial_world.world.scales[scale_index];

				for (godot::Vector3i& pos : scale_commands.nodes_load)
				{
					scale.nodes.try_emplace(pos, spatial_world.world.create_node());
				}

				for (godot::Vector3i& pos : scale_commands.nodes_unload)
				{
					auto it = scale.nodes.find(pos);

					spatial_world.world.destroy_node(it->second);

					scale.nodes.erase(it);
				}

				scale_commands.nodes_load.clear();
				scale_commands.nodes_unload.clear();
			}
		});
	}

	void SpatialWorldLoaderExample(SpatialCoord3D, SpatialNode3D*, SpatialWorld3DComponent&, const SpatialLoader3DComponent&) {};
	void SpatialWorldNodeExample(SpatialNode3D*, SpatialWorld3DComponent&, const SpatialNode3DComponent&) {};
	void SpatialWorldRegionExample(SpatialNode3D*, SpatialWorld3DComponent&, const SpatialRegion3DComponent&) {};
	void SpatialWorldScaleExample(SpatialNode3D*, SpatialWorld3DComponent&, const SpatialScale3DComponent&) {};
	void SpatialWorldExample(SpatialNode3D* node, SpatialWorld3DComponent&) {};

	SpatialModule::SpatialModule(flecs::world& world)
	{
		world.module<SpatialModule>("SpatialModule");

		world.import<SpatialComponents>();
		world.import<PhysicsComponents>();
		world.import<SimulationComponents>();

		// Spatial world loader systems
		world.system("WorldLoaderProgressSync")
			.no_readonly()
			.kind<WorldLoaderProgressPhase>()
			.iter([](flecs::iter& it) {});

		world.system<SpatialWorld3DComponent, const SpatialLoader3DComponent>("SpatialWorldLoaderExample")
			.multi_threaded()
			.kind<WorldLoaderProgressPhase>()
			.term<ParallelWorkerComponent>()
			.term_at(1).parent()
			.each(Loader3DParallelSystem<&SpatialWorldLoaderExample>);

		world.system<SpatialWorld3DComponent, const SpatialLoader3DComponent, SpatialCommands3DComponent, const SimulationGlobal>("SpatialWorldLoaderUpdateNodes")
			.multi_threaded()
			.kind<WorldLoaderProgressPhase>()
			.term_at(1).parent()
			.term_at(4).src<SimulationGlobal>()
			.each(Loader3DParallelSystem<&SpatialWorldLoaderUpdateNodes, SpatialCommands3DComponent&, const SimulationGlobal&>);

		// Spatial world node systems
		world.system("WorldNodeProgressSync")
			.no_readonly()
			.kind<WorldNodeProgressPhase>()
			.iter([](flecs::iter& it) {});

		world.system<SpatialWorld3DComponent, const SpatialNode3DComponent>("SpatialWorldNodeExample")
			.multi_threaded()
			.kind<WorldNodeProgressPhase>()
			.term<ParallelWorkerComponent>()
			.term_at(1).parent()
			.each(Node3DParallelSystem<&SpatialWorldNodeExample>);

		// Spatial world region systems
		world.system("WorldRegionProgressSync")
			.no_readonly()
			.kind<WorldRegionProgressPhase>()
			.iter([](flecs::iter& it) {});

		world.system<SpatialWorld3DComponent, const SpatialRegion3DComponent>("SpatialWorldRegionExample")
			.multi_threaded()
			.kind<WorldRegionProgressPhase>()
			.term<ParallelWorkerComponent>()
			.term_at(1).parent()
			.each(Region3DParallelSystem<&SpatialWorldRegionExample>);

		// Spatial world scale systems
		world.system("WorldScaleProgressSync")
			.no_readonly()
			.kind<WorldScaleProgressPhase>()
			.iter([](flecs::iter& it) {});

		world.system<SpatialWorld3DComponent, const SpatialScale3DComponent>("SpatialWorldScaleExample")
			.multi_threaded()
			.kind<WorldScaleProgressPhase>()
			.term<ParallelWorkerComponent>()
			.term_at(1).parent()
			.each(Scale3DParallelSystem<&SpatialWorldScaleExample>);

		world.system<SpatialWorld3DComponent, const SpatialScale3DComponent, SpatialCommands3DComponent, const SimulationGlobal>("SpatialWorldUnloadUnusedNodes")
			.multi_threaded()
			.kind<WorldScaleProgressPhase>()
			.term<ParallelWorkerComponent>()
			.term_at(1).parent()
			.term_at(4).src<SimulationGlobal>()
			.each(Scale3DParallelSystem<&SpatialWorldUnloadUnusedNodes, SpatialCommands3DComponent&, const SimulationGlobal&>);

		// Spatial world scale
		world.system("WorldProgressSync")
			.no_readonly()
			.kind<WorldProgressPhase>()
			.iter([](flecs::iter& it) {});

		world.system<SpatialWorld3DComponent>("SpatialWorldExample")
			.multi_threaded()
			.kind<WorldProgressPhase>()
			.each(World3DParallelSystem<&SpatialWorldExample>);

		world.system<SpatialWorld3DComponent>("SpatialWorldApplyCommands")
			.multi_threaded()
			.kind<WorldProgressPhase>()
			.each(SpatialWorldApplyCommands);
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