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

	void InitializeSpatialNode(SpatialNode3D& node, SpatialScale3D& scale, SpatialScale3D& parent_scale)
	{
		for (uint8_t neighbour_index = 0; neighbour_index < 6; neighbour_index++)
		{
			godot::Vector3i neighbour_pos = node.coord.pos + node_neighbour_offsets[neighbour_index];

			auto it = scale.nodes.find(neighbour_pos);

			if (it == scale.nodes.end())
			{
				continue;
			}

			SpatialNode3D* neighbour_node = it->second.get();

			node.neighbours[neighbour_index] = neighbour_node;
			node.neighbour_mask |= 1 << neighbour_index;

			neighbour_node->neighbours[5 - neighbour_index] = &node;
			neighbour_node->neighbour_mask |= 1 << (5 - neighbour_index);
		}

		{
			godot::Vector3i parent_pos = node.coord.pos / 2;

			auto it = parent_scale.nodes.find(parent_pos);

			if (it == parent_scale.nodes.end())
			{
				return;
			}

			SpatialNode3D* parent_node = it->second.get();

			node.parent = parent_node;
			node.parent_index = GetNodeParentIndex(node.coord.pos);

			parent_node->children_array[node.parent_index] = &node;
			parent_node->children_mask |= 1 << node.parent_index;
		}
	}

	void UninitializeSpatialNode(SpatialNode3D& node, SpatialScale3D& scale, SpatialScale3D& parent_scale)
	{
		for (uint8_t neighbour_index = 0; neighbour_index < 6; neighbour_index++)
		{
			if (SpatialNode3D* neighbour_node = node.neighbours[neighbour_index])
			{
				neighbour_node->neighbours[5 - neighbour_index] = nullptr;
				neighbour_node->neighbour_mask &= ~(1 << (5 - neighbour_index));
			}
		}

		if (SpatialNode3D* parent_node = node.parent)
		{
			parent_node->children_array[node.parent_index] = nullptr;
			parent_node->children_mask &= ~(1 << node.parent_index);
		}
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

		// System to keep alive all nodes around a loader and request the loading of any missing
		world.system<SpatialWorld3DComponent, const SpatialLoader3DComponent, const SimulationGlobal>("SpatialLoaderKeepAliveNodes")
			.multi_threaded()
			.kind<WorldLoaderWorkerPhase>()
			.term_at(1).parent()
			.term_at(3).src<SimulationGlobal>()
			.each([](SpatialWorld3DComponent& spatial_world, const SpatialLoader3DComponent& spatial_loader, const SimulationGlobal& world_time)
		{
			PARALLEL_ACCESS(spatial_world, world_time);

			SpatialLoader3DNodeProcessor(spatial_world, spatial_loader, [&world_time](SpatialCoord3D coord, SpatialNode3D* node)
			{
				PARALLEL_ACCESS(node);

				if (node != nullptr)
				{
					node->last_update_time = world_time.frame_start;
				}
			});
		});

		world.system<SpatialWorld3DComponent, const SpatialScale3DWorkerComponent>("SpatialScaleCreateNodes")
			.multi_threaded()
			.kind<WorldScaleWorkerPhase>()
			.term_at(1).parent()
			.interval(0.05)
			.each([](flecs::entity entity, SpatialWorld3DComponent& spatial_world, const SpatialScale3DWorkerComponent& scale_worker)
		{
			PARALLEL_ACCESS(spatial_world);

			uint8_t scale_index = scale_worker.scale;

			SpatialScale3D& scale = spatial_world.scales[scale_index];

			SpatialScaleNodeCommands& scale_load_commands = spatial_world.load_commands[scale_index];

			flecs::query<const SpatialLoader3DComponent> staged_loaders_query(entity.world(), spatial_world.loaders_query);

			// For each command list that is a child of the world
			staged_loaders_query.each([&spatial_world, scale_index, &scale, &scale_load_commands](const SpatialLoader3DComponent& spatial_loader)
			{
				PARALLEL_ACCESS(spatial_loader);

				ForEachCoordInSphere(spatial_loader.coord.pos, spatial_loader.dist_per_lod, [&spatial_world, scale_index, &scale, &scale_load_commands](godot::Vector3i pos)
				{
					auto it = scale.nodes.find(pos);

					if (it == scale.nodes.end())
					{
						SpatialCoord3D coord{ pos, scale_index };
						SpatialNode3D* node = it->second.get();

						scale.nodes.emplace(coord.pos, spatial_world.create_node());
						scale_load_commands.push_back(coord.pos);
					}
				});
			});
		});

		// System to mark any nodes that are no longer being observed by any loader to be unloaded
		world.system<SpatialWorld3DComponent, const SpatialScale3DWorkerComponent, const SimulationGlobal>("SpatialScaleUnloadUnusedNodes")
			.multi_threaded()
			.kind<WorldScaleWorkerPhase>()
			.term_at(1).parent()
			.term_at(3).src<SimulationGlobal>()
			.each([](SpatialWorld3DComponent& spatial_world, const SpatialScale3DWorkerComponent& scale_worker, const SimulationGlobal& world_time)
		{
			PARALLEL_ACCESS(spatial_world, world_time);

			SpatialScaleNodeCommands& scale_unload_commands = spatial_world.unload_commands[scale_worker.scale];

			SpatialScale3DNodeProcessor(spatial_world, scale_worker, [&world_time, &scale_unload_commands](SpatialNode3D* node)
			{
				if (world_time.frame_start - node->last_update_time > 20s)
				{
					scale_unload_commands.push_back(node->coord.pos);
				}
			});
		});

		// System to initialize spatial nodes that have been added
		world.system<SpatialWorld3DComponent>("SpatialWorldLoadNodes")
			.multi_threaded()
			.kind<WorldWorkerPhase>()
			.each([](flecs::entity entity, SpatialWorld3DComponent& spatial_world)
		{
			for (uint8_t scale_index = 0; scale_index < spatial_world.max_scale - 1; scale_index++)
			{
				SpatialScale3D& scale = spatial_world.scales[scale_index];
				SpatialScale3D& parent_scale = spatial_world.scales[scale_index + 1];

				SpatialScaleNodeCommands& scale_load_commands = spatial_world.load_commands[scale_index];

				for (godot::Vector3i pos : scale_load_commands)
				{
					auto it = scale.nodes.find(pos);

					if (it == scale.nodes.end())
					{
						continue;
					}

					SpatialNode3D& node = *it->second;

					InitializeSpatialNode(node, scale, parent_scale);

					for (SpatialNodeProcessCB& command_processor : spatial_world.load_command_processors)
					{
						command_processor(node);
					}
				}

				scale_load_commands.clear();
			}
		});

		// System to uninitialize and delete spatial nodes that have been marked to unload
		world.system<SpatialWorld3DComponent>("SpatialWorldUnloadNodes")
			.multi_threaded()
			.kind<WorldWorkerPhase>()
			.each([](flecs::entity entity, SpatialWorld3DComponent& spatial_world)
		{
			for (uint8_t scale_index = 0; scale_index < spatial_world.max_scale - 1; scale_index++)
			{
				SpatialScale3D& scale = spatial_world.scales[scale_index];
				SpatialScale3D& parent_scale = spatial_world.scales[scale_index + 1];

				SpatialScaleNodeCommands& scale_unload_commands = spatial_world.unload_commands[scale_index];

				for (godot::Vector3i pos : scale_unload_commands)
				{
					auto it = scale.nodes.find(pos);

					if (it == scale.nodes.end())
					{
						continue;
					}

					SpatialNode3D& node = *it->second;

					for (SpatialNodeProcessCB& command_processor : spatial_world.unload_command_processors)
					{
						command_processor(node);
					}

					UninitializeSpatialNode(node, scale, parent_scale);

					spatial_world.destroy_node(it->second);

					scale.nodes.erase(it);
				}

				scale_unload_commands.clear();
			}
		});
	}

	void SpatialModule::AddSpatialScaleWorkers(flecs::world& world, flecs::entity_t spatial_world_entity)
	{
		const SpatialWorld3DComponent* spatial_world = flecs::entity(world, spatial_world_entity).get<SpatialWorld3DComponent>();

		DEBUG_ASSERT(spatial_world != nullptr, "The entity should have a spatial world to add spatial workers");

		for (uint8_t scale_index = 0; scale_index < spatial_world->max_scale; scale_index++)
		{
			flecs::entity scale_worker_entity = world.entity()
				.child_of(spatial_world_entity)
				.add<SpatialScale3DWorkerComponent>();

			scale_worker_entity.set([scale_index](SpatialScale3DWorkerComponent& scale_worker)
			{
				scale_worker.scale = scale_index;
			});
		}
	}

	void SpatialModule::RemoveSpatialScaleWorkers(flecs::world& world, flecs::entity_t spatial_world_entity)
	{
		world.filter_builder()
			.read<SpatialScale3DWorkerComponent>()
			.read(flecs::ChildOf, spatial_world_entity)
			.each([](flecs::entity entity)
		{
			entity.destruct();
		});
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