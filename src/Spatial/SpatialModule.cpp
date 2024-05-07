#include "SpatialModule.h"
#include "SpatialComponents.h"
#include "SpatialProcessors.h"

#include "Simulation/SimulationComponents.h"

#include "Util/Debug.h"
#include "Util/Callback.h"

namespace voxel_game
{
	const godot::Vector3i node_child_offsets[8] =
	{
		{0, 0, 0},
		{0, 0, 1},
		{0, 1, 0},
		{0, 1, 1},
		{1, 0, 0},
		{1, 0, 1},
		{1, 1, 0},
		{1, 1, 1},
	};

	const godot::Vector3i node_neighbour_offsets[6] =
	{
		{0, 0, -1},
		{0, 0, 1},
		{0, -1, 0},
		{0, 1, 0},
		{-1, 0, 0},
		{1, 0, 0},
	};

	uint8_t GetNodeParentIndex(godot::Vector3i pos)
	{
		pos %= 2;
		return (pos.x) + (pos.y * 2) + (pos.z * 4);
	}

	void InitializeSpatialNode(godot::Vector3i node_pos, SpatialScale3D& scale, SpatialScale3D& parent_scale)
	{
		auto it = scale.nodes.find(node_pos);

		if (it == scale.nodes.end())
		{
			return;
		}

		SpatialNode3D* node = it->second.get();

		for (uint8_t neighbour_index = 0; neighbour_index < 6; neighbour_index++)
		{
			godot::Vector3i neighbour_pos = node_pos + node_neighbour_offsets[neighbour_index];

			auto it = scale.nodes.find(neighbour_pos);

			if (it == scale.nodes.end())
			{
				continue;
			}

			SpatialNode3D* neighbour_node = it->second.get();

			node->neighbours[neighbour_index] = neighbour_node;
			node->neighbour_mask |= 1 << neighbour_index;

			neighbour_node->neighbours[5 - neighbour_index] = node;
			neighbour_node->neighbour_mask |= 1 << (5 - neighbour_index);
		}

		{
			godot::Vector3i parent_pos = node_pos / 2;

			auto it = parent_scale.nodes.find(parent_pos);

			if (it == parent_scale.nodes.end())
			{
				return;
			}

			SpatialNode3D* parent_node = it->second.get();

			node->parent = parent_node;
			node->parent_index = GetNodeParentIndex(node_pos);

			parent_node->children_array[node->parent_index] = node;
			parent_node->children_mask |= 1 << node->parent_index;
		}
	}

	void SpatialWorldInitializeNodes(flecs::entity entity, SpatialWorld3DComponent& spatial_world)
	{
		for (uint8_t scale_index = 0; scale_index < spatial_world.max_scale - 1; scale_index++)
		{
			SpatialScale3D& scale = spatial_world.scales[scale_index];
			SpatialScale3D& parent_scale = spatial_world.scales[scale_index + 1];

			flecs::query<SpatialLoadCommands3DComponent> load_commands_query(entity.world(), spatial_world.load_commands_query);

			load_commands_query.each([scale_index, &scale, &parent_scale](SpatialLoadCommands3DComponent& load_commands)
			{
				std::vector<godot::Vector3i>& scale_commands = load_commands.scales[scale_index];

				for (godot::Vector3i pos : scale_commands)
				{
					InitializeSpatialNode(pos, scale, parent_scale);
				}

				scale_commands.clear();
			});
		}
	}

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

		world.system("WorldLoaderWorkerSync")
			.no_readonly()
			.kind<WorldLoaderWorkerPhase>()
			.iter([](flecs::iter& it) {});

		world.system("WorldNodeWorkerSync")
			.no_readonly()
			.kind<WorldNodeWorkerPhase>()
			.iter([](flecs::iter& it) {});

		world.system("WorldRegionWorkerSync")
			.no_readonly()
			.kind<WorldRegionWorkerPhase>()
			.iter([](flecs::iter& it) {});

		world.system("WorldScaleWorkerSync")
			.no_readonly()
			.kind<WorldScaleWorkerPhase>()
			.iter([](flecs::iter& it) {});

		world.system("WorldWorkerSync")
			.no_readonly()
			.kind<WorldWorkerPhase>()
			.iter([](flecs::iter& it) {});

		// Systems

		world.system<SpatialWorld3DComponent>("SpatialWorldInitializeNodes")
			.multi_threaded()
			.kind<WorldWorkerPhase>()
			.each(SpatialWorldInitializeNodes);

		world.system<SpatialWorld3DComponent, const SpatialLoader3DComponent, const SimulationGlobal>("LoaderKeepAliveNodes")
			.multi_threaded()
			.kind<WorldLoaderWorkerPhase>()
			.term_at(1).parent()
			.term_at(3).src<SimulationGlobal>()
			.each(LoaderKeepAliveNodes);

		world.system<SpatialWorld3DComponent, const SpatialScale3DWorkerComponent, SpatialUnloadCommands3DComponent, const SimulationGlobal>("SpatialWorldUnloadUnusedNodes")
			.multi_threaded()
			.kind<WorldScaleWorkerPhase>()
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