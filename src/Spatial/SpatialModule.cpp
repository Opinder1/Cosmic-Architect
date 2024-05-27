#include "SpatialModule.h"
#include "SpatialComponents.h"
#include "SpatialTraverse.h"
#include "SpatialCommands.h"

#include "Simulation/SimulationComponents.h"

#include "Util/Debug.h"
#include "Util/Callback.h"
#include "Util/VariableLengthArray.h"

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
		for (size_t neighbour_index = 0; neighbour_index < 6; neighbour_index++)
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

		if (&scale != &parent_scale)
		{
			godot::Vector3i parent_pos = node.coord.pos / 2;

			auto it = parent_scale.nodes.find(parent_pos);

			DEBUG_ASSERT(it != parent_scale.nodes.end(), "The parent node should already exist if we try to load this node");

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

		if (&scale != &parent_scale)
		{
			if (SpatialNode3D* parent_node = node.parent)
			{
				parent_node->children_array[node.parent_index] = nullptr;
				parent_node->children_mask &= ~(1 << node.parent_index);
			}
		}
	}

	SpatialModule::SpatialModule(flecs::world& world)
	{
		world.module<SpatialModule>(DEBUG_ONLY("SpatialModule"));

		world.import<SpatialComponents>();
		world.import<SimulationComponents>();

		// Systems

		// System to keep alive all nodes around a loader and request the loading of any missing
		world.system<SpatialWorld3DComponent, const SpatialLoader3DComponent, const SimulationGlobal>(DEBUG_ONLY("SpatialLoaderKeepAliveNodes"))
			.multi_threaded()
			.kind<WorldLoaderWorkerPhase>()
			.term_at(1).parent()
			.term_at(3).src<SimulationGlobal>()
			.each([](SpatialWorld3DComponent& spatial_world, const SpatialLoader3DComponent& spatial_loader, const SimulationGlobal& world_time)
		{
			PARALLEL_ACCESS(spatial_world, world_time);

			for (uint8_t scale_index = spatial_loader.min_lod; scale_index < spatial_loader.max_lod; scale_index++)
			{
				const SpatialScale3D& spatial_scale = spatial_world.scales[scale_index];

				ForEachCoordInSphere(spatial_loader.coord.pos, spatial_loader.dist_per_lod, [&spatial_scale, &world_time](godot::Vector3i pos)
				{
					auto it = spatial_scale.nodes.find(pos);

					if (it == spatial_scale.nodes.end())
					{
						return;
					}

					SpatialNode3D& node = *it->second;

					node.last_update_time = world_time.frame_start;
				});
			}
		});

		// System to run all loaders in each scale in parallel
		world.system<SpatialWorld3DComponent, const SpatialScale3DWorkerComponent>(DEBUG_ONLY("SpatialLoaderLoadScaleNodes"))
			.multi_threaded()
			.kind<WorldScaleWorkerPhase>()
			.term_at(1).parent()
			.interval(0.05)
			.each([](flecs::entity entity, SpatialWorld3DComponent& spatial_world, const SpatialScale3DWorkerComponent& scale_worker)
		{
			PARALLEL_ACCESS(spatial_world);

			size_t scale_index = scale_worker.scale;
			SpatialScale3D& scale = spatial_world.scales[scale_index];

			flecs::query<const SpatialLoader3DComponent> staged_loaders_query(entity.world(), spatial_world.loaders_query);

			// For each command list that is a child of the world
			staged_loaders_query.each([&scale](const SpatialLoader3DComponent& spatial_loader)
			{
				PARALLEL_ACCESS(spatial_loader);

				ForEachCoordInSphere(spatial_loader.coord.pos, spatial_loader.dist_per_lod, [&scale](godot::Vector3i pos)
				{
					auto it = scale.nodes.find(pos);

					if (it == scale.nodes.end())
					{
						scale.load_commands.push_back(pos);
					}
				});
			});
		});

		// System to mark any nodes that are no longer being observed by any loader to be unloaded
		world.system<SpatialWorld3DComponent, const SpatialScale3DWorkerComponent, const SimulationGlobal>(DEBUG_ONLY("SpatialScaleUnloadUnusedNodes"))
			.multi_threaded()
			.kind<WorldScaleWorkerPhase>()
			.term_at(1).parent()
			.term_at(3).src<SimulationGlobal>()
			.each([](SpatialWorld3DComponent& spatial_world, const SpatialScale3DWorkerComponent& scale_worker, const SimulationGlobal& world_time)
		{
			PARALLEL_ACCESS(spatial_world, world_time);

			size_t scale_index = scale_worker.scale;
			SpatialScale3D& scale = spatial_world.scales[scale_index];

			for (auto&& [pos, node] : scale.nodes)
			{
				if (world_time.frame_start - node->last_update_time > 20s)
				{
					scale.unload_commands.push_back(node->coord.pos);
				}
			}
		});

		// System to initialize spatial nodes that have been added
		world.system<SpatialWorld3DComponent>(DEBUG_ONLY("SpatialWorldInitializeNodes"))
			.multi_threaded()
			.kind<WorldCreatePhase>()
			.each([](SpatialWorld3DComponent& spatial_world)
		{
			// Initialize the largest scales first
			for (size_t scale_index = spatial_world.max_scale; scale_index --> 0;)
			{
				SpatialScale3D& scale = spatial_world.scales[scale_index];
				uint8_t parent_scale_index = std::min(scale_index + 1, spatial_world.max_scale - 1);
				SpatialScale3D& parent_scale = spatial_world.scales[parent_scale_index];

				for (godot::Vector3i pos : scale.load_commands)
				{
					DEBUG_ASSERT(scale.nodes.find(pos) == scale.nodes.end(), "The load command is trying to load a node that already has been loaded");

					auto&& [it, success] = scale.nodes.emplace(pos, spatial_world.node_builder.node_create());
					DEBUG_ASSERT(success, "The node should have been emplaced");

					SpatialNode3D& node = *it->second;

					InitializeSpatialNode(node, scale, parent_scale);
				}
			}
		});

		// System to delete spatial nodes that have been marked to unload
		world.system<SpatialWorld3DComponent>(DEBUG_ONLY("SpatialWorldDestroyNodes"))
			.multi_threaded()
			.kind<WorldDestroyPhase>()
			.each([](SpatialWorld3DComponent& spatial_world)
		{
			for (size_t scale_index = 0; scale_index < spatial_world.max_scale - 1; scale_index++)
			{
				SpatialScale3D& scale = spatial_world.scales[scale_index];
				uint8_t parent_scale_index = std::min(scale_index + 1, spatial_world.max_scale - 1);
				SpatialScale3D& parent_scale = spatial_world.scales[parent_scale_index];

				for (godot::Vector3i pos : scale.unload_commands)
				{
					auto it = scale.nodes.find(pos);
					DEBUG_ASSERT(it != scale.nodes.end(), "We should have only sent unload commands for existing nodes");

					SpatialNode3D& node = *it->second;

					UninitializeSpatialNode(node, scale, parent_scale);

					spatial_world.node_builder.node_destroy(it->second);

					scale.nodes.erase(it);
				}

				scale.unload_commands.clear();
			}
		});

		// System to load spatial nodes that have been added
		world.system<SpatialWorld3DComponent, const SpatialScale3DWorkerComponent>(DEBUG_ONLY("SpatialWorldProcessLoadNodeCommands"))
			.multi_threaded()
			.kind<WorldLoadPhase>()
			.term_at(1).parent()
			.each([](flecs::entity entity, SpatialWorld3DComponent& spatial_world, const SpatialScale3DWorkerComponent& scale_worker)
		{
			PARALLEL_ACCESS(spatial_world);

			size_t scale_index = scale_worker.scale;
			SpatialScale3D& scale = spatial_world.scales[scale_index];
			std::vector<SpatialNodeCommandProcessorBase>& processors = spatial_world.load_command_processors;

			if (scale.load_commands.empty()) // Don't continue if there aren't any commands
			{
				return;
			}
			
			if (processors.empty()) // Don't continue if there aren't any processors but make sure to clear the commands
			{
				scale.load_commands.clear();
				return;
			}

			VariableLengthArray<void*> states = MakeVariableLengthArray(void*, processors.size());

			for (size_t i = 0; i < states.size(); i++)
			{
				states[i] = alloca(processors[i].state_size);

				processors[i].state_initialize(states[i], entity, spatial_world);
			}

			for (godot::Vector3i pos : scale.load_commands)
			{
				auto it = scale.nodes.find(pos);
				DEBUG_ASSERT(it != scale.nodes.end(), "The node should have been initialized");
				DEBUG_ASSERT(it->second, "The node should have been initialized with data");

				SpatialNode3D& node = *it->second;

				for (size_t i = 0; i < states.size(); i++)
				{
					processors[i].process(states[i], scale, node);
				}
			}

			for (size_t i = 0; i < states.size(); i++)
			{
				processors[i].state_destroy(states[i]);
			}

			scale.load_commands.clear();
		});

		// System to unload spatial nodes that have been marked to unload
		world.system<SpatialWorld3DComponent, const SpatialScale3DWorkerComponent>(DEBUG_ONLY("SpatialWorldProcessUnloadNodeCommands"))
			.multi_threaded()
			.kind<WorldUnloadPhase>()
			.term_at(1).parent()
			.each([](flecs::entity entity, SpatialWorld3DComponent& spatial_world, const SpatialScale3DWorkerComponent& scale_worker)
		{
			PARALLEL_ACCESS(spatial_world);

			size_t scale_index = scale_worker.scale;
			SpatialScale3D& scale = spatial_world.scales[scale_index];
			std::vector<SpatialNodeCommandProcessorBase>& processors = spatial_world.unload_command_processors;

			if (scale.unload_commands.empty()) // Don't continue if there aren't any commands
			{
				return;
			}

			if (processors.empty()) // Don't continue if there aren't any processors but make sure to clear the commands
			{
				scale.unload_commands.clear();
				return;
			}

			VariableLengthArray<void*> states = MakeVariableLengthArray(void*, processors.size());

			for (size_t i = 0; i < states.size(); i++)
			{
				states[i] = alloca(processors[i].state_size);

				processors[i].state_initialize(states[i], entity, spatial_world);
			}

			for (godot::Vector3i pos : scale.unload_commands)
			{
				auto it = scale.nodes.find(pos);
				DEBUG_ASSERT(it != scale.nodes.end(), "We should have only sent unload commands for existing nodes");

				SpatialNode3D& node = *it->second;

				for (size_t i = 0; i < states.size(); i++)
				{
					processors[i].process(states[i], scale, node);
				}
			}

			for (size_t i = 0; i < states.size(); i++)
			{
				processors[i].state_destroy(states[i]);
			}
		});

		// System to unload spatial nodes that have been marked to unload
		world.system<SpatialWorld3DComponent, const SpatialScale3DWorkerComponent>(DEBUG_ONLY("SpatialWorldProcessTickCommands"))
			.multi_threaded()
			.kind<WorldUnloadPhase>()
			.term_at(1).parent()
			.each([](flecs::entity entity, SpatialWorld3DComponent& spatial_world, const SpatialScale3DWorkerComponent& scale_worker)
		{
			PARALLEL_ACCESS(spatial_world);

			size_t scale_index = scale_worker.scale;
			SpatialScale3D& scale = spatial_world.scales[scale_index];
			std::vector<SpatialNodeCommandProcessorBase>& processors = spatial_world.tick_command_processors;

			if (scale.tick_commands.empty()) // Don't continue if there aren't any commands
			{
				return;
			}

			if (processors.empty()) // Don't continue if there aren't any processors but make sure to clear the commands
			{
				scale.tick_commands.clear();
				return;
			}

			VariableLengthArray<void*> states = MakeVariableLengthArray(void*, processors.size());

			for (size_t i = 0; i < states.size(); i++)
			{
				states[i] = alloca(processors[i].state_size);

				processors[i].state_initialize(states[i], entity, spatial_world);
			}

			for (godot::Vector3i pos : scale.tick_commands)
			{
				auto it = scale.nodes.find(pos);
				DEBUG_ASSERT(it != scale.nodes.end(), "We should have only sent unload commands for existing nodes");

				SpatialNode3D& node = *it->second;

				for (size_t i = 0; i < states.size(); i++)
				{
					processors[i].process(states[i], scale, node);
				}
			}

			for (size_t i = 0; i < states.size(); i++)
			{
				processors[i].state_destroy(states[i]);
			}
		});
	}

	void SpatialModule::AddSpatialScaleWorkers(flecs::entity spatial_world_entity)
	{
		flecs::world world(spatial_world_entity.world());

		const SpatialWorld3DComponent* spatial_world = spatial_world_entity.get<SpatialWorld3DComponent>();

		DEBUG_ASSERT(spatial_world != nullptr, "The entity should have a spatial world to add spatial workers");

		for (size_t scale_index = 0; scale_index < spatial_world->max_scale; scale_index++)
		{
			flecs::entity scale_worker_entity = world.entity()
				.child_of(spatial_world_entity)
				.add<SpatialScale3DWorkerComponent>()
				.set([scale_index](SpatialScale3DWorkerComponent& scale_worker)
			{
				scale_worker.scale = scale_index;
			});
		}
	}

	void SpatialModule::RemoveSpatialScaleWorkers(flecs::entity spatial_world_entity)
	{
		flecs::world world(spatial_world_entity.world());

		world.filter_builder()
			.read(flecs::ChildOf, spatial_world_entity)
			.read<SpatialScale3DWorkerComponent>()
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