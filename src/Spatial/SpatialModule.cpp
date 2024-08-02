#include "SpatialModule.h"
#include "SpatialComponents.h"
#include "SpatialTraverse.h"
#include "SpatialCommands.h"

#include "Simulation/SimulationComponents.h"

#include "Physics/PhysicsComponents.h"

#include "Util/Debug.h"
#include "Util/Callback.h"

namespace voxel_game
{
	const godot::Vector3i node_neighbour_offsets[6] =
	{
		{0, 0, -1},
		{0, 0, 1},
		{0, -1, 0},
		{0, 1, 0},
		{-1, 0, 0},
		{1, 0, 0},
	};

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

	uint8_t GetNodeParentIndex(godot::Vector3i pos)
	{
		return (pos.x) + (pos.y * 2) + (pos.z * 4);
	}

	template<typename PhaseT, typename DependT>
	void CreateSyncedPhase(flecs::world& world)
	{
		flecs::entity phase = world.entity<PhaseT>()
			.add(flecs::Phase);

		flecs::scoped_world scope = phase.scope();

		flecs::entity sync_phase = scope.entity(DEBUG_ONLY("ThreadSyncPhase"))
			.add(flecs::Phase)
			.depends_on<DependT>();

		scope.system(DEBUG_ONLY("ThreadSyncSystem"))
			.immediate()
			.kind(sync_phase)
			.run([&world](flecs::iter& it)
		{
			DEBUG_THREAD_CHECK_SYNC(&world);
		});

		phase.depends_on(sync_phase);
	}

	void InitializeSpatialNode(SpatialNode3D& node, SpatialWorld3DComponent& spatial_world, uint8_t scale_index)
	{
		SpatialScale3D& scale = *spatial_world.scales[scale_index];

		for (uint8_t neighbour_index = 0; neighbour_index < 6; neighbour_index++)
		{
			godot::Vector3i neighbour_pos = node.coord.pos + node_neighbour_offsets[neighbour_index];

			SpatialNodeMap::iterator it = scale.nodes.find(neighbour_pos);

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

		if (scale_index < spatial_world.max_scale - 1)
		{
			SpatialScale3D& parent_scale = *spatial_world.scales[scale_index + 1];

			SpatialCoord3D parent_pos = node.coord.GetParent();

			SpatialNodeMap::iterator it = parent_scale.nodes.find(parent_pos.pos);

			if (it != parent_scale.nodes.end())
			{
				SpatialNode3D& parent_node = *it->second;

				node.parent = &parent_node;
				node.parent_index = GetNodeParentIndex(node.coord.GetParentRelPos());

				DEBUG_ASSERT(node.parent_index < 8, "The parent index is out of range");

				parent_node.children_array[node.parent_index] = &node;
				parent_node.children_mask |= 1 << node.parent_index;
			}
		}

		if (scale_index > 0)
		{
			SpatialScale3D& child_scale = *spatial_world.scales[scale_index - 1];

			for (uint8_t child_index = 0; child_index < 8; child_index++)
			{
				SpatialCoord3D child_pos = node.coord.GetBottomLeftChild();
				child_pos.pos += node_child_offsets[child_index];

				SpatialNodeMap::iterator it = child_scale.nodes.find(child_pos.pos);

				if (it != child_scale.nodes.end())
				{
					SpatialNode3D& child_node = *it->second;

					child_node.parent = &node;
					child_node.parent_index = child_index;

					node.children_array[child_index] = &child_node;
					node.children_mask |= 1 << child_index;
				}
			}
		}

		node.initialized = true;
	}

	void UninitializeSpatialNode(SpatialNode3D& node, SpatialWorld3DComponent& spatial_world, uint8_t scale_index)
	{
		for (uint8_t neighbour_index = 0; neighbour_index < 6; neighbour_index++)
		{
			if (SpatialNode3D* neighbour_node = node.neighbours[neighbour_index])
			{
				neighbour_node->neighbours[5 - neighbour_index] = nullptr;
				neighbour_node->neighbour_mask &= ~(1 << (5 - neighbour_index));
			}
		}

		if (scale_index < spatial_world.max_scale - 1)
		{
			if (SpatialNode3D* parent_node = node.parent)
			{
				parent_node->children_array[node.parent_index] = nullptr;
				parent_node->children_mask &= ~(1 << node.parent_index);
			}
		}

		if (scale_index > 0)
		{
			for (uint8_t child_index = 0; child_index < 8; child_index++)
			{
				if (SpatialNode3D* child_node = node.children_array[child_index])
				{
					child_node->parent = nullptr;
					child_node->parent_index = k_node_no_parent;
				}
			}
		}

		node.initialized = false;
	}

	SpatialModule::SpatialModule(flecs::world& world)
	{
		world.module<SpatialModule>();

		world.import<SpatialComponents>();
		world.import<SimulationComponents>();
		world.import<PhysicsComponents>();

		// Relationships

		world.singleton<SpatialEntity3DComponent>()
			.add_second<SpatialWorld3DComponent>(flecs::OneOf)
			.add_second<Position3DComponent>(flecs::With);

		world.singleton<SpatialScale3DWorkerComponent>()
			.add_second<SpatialWorld3DComponent>(flecs::OneOf);

		world.singleton<SpatialRegion3DWorkerComponent>()
			.add_second<SpatialWorld3DComponent>(flecs::OneOf);

		world.singleton<SpatialLoader3DComponent>()
			.add_second<SpatialEntity3DComponent>(flecs::With);

		// Phases

		// The phase that all spatial wrold processing happens in
		world.singleton<SpatialWorldMultithreadPhase>()
			.add(flecs::Phase)
			.depends_on(flecs::OnUpdate);

		CreateSyncedPhase<WorldRegionWorkerPhase, SpatialWorldMultithreadPhase>(world);
		CreateSyncedPhase<WorldScaleWorkerPhase, WorldRegionWorkerPhase>(world);
		CreateSyncedPhase<WorldWorkerPhase, WorldScaleWorkerPhase>(world);
		CreateSyncedPhase<WorldCreatePhase, WorldWorkerPhase>(world);
		CreateSyncedPhase<WorldLoadPhase, WorldCreatePhase>(world);
		CreateSyncedPhase<WorldUnloadPhase, WorldLoadPhase>(world);
		CreateSyncedPhase<WorldDestroyPhase, WorldUnloadPhase>(world);
		CreateSyncedPhase<WorldEndPhase, WorldDestroyPhase>(world);

		// Observers

		// Add a cached query for all of a spatial worlds child nodes for fast access
		world.observer<SpatialWorld3DComponent>(DEBUG_ONLY("AddWorldQueries"))
			.event(flecs::OnAdd)
			.each([](flecs::entity world_entity, SpatialWorld3DComponent& spatial_world)
		{
			flecs::scoped_world scope = world_entity.scope(); // Add the queries as children of the entity so they are automatically destructed

			spatial_world.entities_query = scope.query_builder<const SpatialEntity3DComponent>(DEBUG_ONLY("WorldEntitiesQuery"))
				.with(flecs::ChildOf, world_entity).read() // Use read() as its required for queries run inside systems
				.cached()
				.build().c_ptr();

			spatial_world.scale_workers_query = scope.query_builder<const SpatialScale3DWorkerComponent>(DEBUG_ONLY("WorldScaleWorkersQuery"))
				.with(flecs::ChildOf, world_entity).read() // Use read() as its required for queries run inside systems
				.cached()
				.build().c_ptr();

			spatial_world.region_workers_query = scope.query_builder<const SpatialRegion3DWorkerComponent>(DEBUG_ONLY("WorldRegionWorkersQuery"))
				.with(flecs::ChildOf, world_entity).read() // Use read() as its required for queries run inside systems
				.cached()
				.build().c_ptr();

			spatial_world.loaders_query = scope.query_builder<const SpatialLoader3DComponent>(DEBUG_ONLY("WorldLoadersQuery"))
				.with(flecs::ChildOf, world_entity).read() // Use read() as its required for queries run inside systems
				.cached()
				.build().c_ptr();
		});

		// Systems

		// System to mark any nodes that are no longer being observed to be unloaded
		world.system<const SpatialScale3DWorkerComponent, SpatialWorld3DComponent, const SimulationTime>(DEBUG_ONLY("ScaleUnloadUnusedNodes"))
			.multi_threaded()
			.interval(0.05)
			.kind<WorldScaleWorkerPhase>()
			.term_at(1).parent()
			.term_at(2).src<SimulationTime>()
			.each([&world](const SpatialScale3DWorkerComponent& scale_worker, SpatialWorld3DComponent& spatial_world, const SimulationTime& world_time)
		{
			DEBUG_THREAD_CHECK_READ(&world, &spatial_world);
			DEBUG_THREAD_CHECK_READ(&world, &world_time);

			size_t scale_index = scale_worker.scale;
			SpatialScale3D& scale = *spatial_world.scales[scale_index];

			DEBUG_THREAD_CHECK_WRITE(&world, &scale);

			for (auto&& [pos, node] : scale.nodes)
			{
				DEBUG_THREAD_CHECK_WRITE(&world, &node);

				if (world_time.frame_start - node->last_update_time > 20s &&
					scale.unload_commands.size() < k_max_frame_unload_commands)
				{
					scale.unload_commands.push_back(node->coord.pos);
				}
			}
		});

		// Systen to create or update all nodes in the range of loaders
		world.system<const SpatialScale3DWorkerComponent, SpatialWorld3DComponent, const SimulationTime>(DEBUG_ONLY("LoaderLoadScaleNodes"))
			.multi_threaded()
			.interval(0.05)
			.kind<WorldScaleWorkerPhase>()
			.term_at(1).parent()
			.term_at(2).src<SimulationTime>()
			.each([&world](flecs::entity worker_entity, const SpatialScale3DWorkerComponent& scale_worker, SpatialWorld3DComponent& spatial_world, const SimulationTime& world_time)
		{
			DEBUG_THREAD_CHECK_READ(&world, &spatial_world);
			DEBUG_THREAD_CHECK_READ(&world, &world_time);

			size_t scale_index = scale_worker.scale;
			SpatialScale3D& scale = *spatial_world.scales[scale_index];

			DEBUG_THREAD_CHECK_WRITE(&world, &scale);

			flecs::query<const SpatialLoader3DComponent> staged_loaders_query(spatial_world.loaders_query);

			// For each command list that is a child of the world
			staged_loaders_query.iter(worker_entity.world()).each([&world, scale_index, &scale, &world_time](const SpatialLoader3DComponent& spatial_loader)
			{
				DEBUG_THREAD_CHECK_READ(&world, &spatial_loader);

				if (scale_index < spatial_loader.min_lod || scale_index > spatial_loader.max_lod)
				{
					return;
				}

				ForEachCoordInSphere(spatial_loader.coord.pos, spatial_loader.dist_per_lod, [&world, &scale, &world_time](godot::Vector3i pos)
				{
					SpatialNodeMap::iterator it = scale.nodes.find(pos);

					if (it != scale.nodes.end())
					{
						SpatialNode3D& node = *it->second;

						DEBUG_THREAD_CHECK_WRITE(&world, &node);

						node.last_update_time = world_time.frame_start;
					}
					else
					{
						DEBUG_ASSERT(std::find(scale.create_commands.begin(), scale.create_commands.end(), pos) == scale.create_commands.end(), "Already exists");
						scale.create_commands.push_back(pos);
					}
				});
			});
		});

		// System to initialize spatial nodes that have been added
		world.system<SpatialWorld3DComponent, const SimulationTime>(DEBUG_ONLY("WorldCreateNodes"))
			.multi_threaded()
			.kind<WorldCreatePhase>()
			.term_at(1).src<SimulationTime>()
			.each([&world](SpatialWorld3DComponent& spatial_world, const SimulationTime& world_time)
		{
			DEBUG_ASSERT(spatial_world.max_scale > 0, "The spatial world should have at least one scale");

			DEBUG_THREAD_CHECK_WRITE(&world, &spatial_world);

			// Initialize the largest scales first
			for (size_t scale_index = spatial_world.max_scale; scale_index-- > 0;)
			{
				SpatialScale3D& scale = *spatial_world.scales[scale_index];

				DEBUG_THREAD_CHECK_WRITE(&world, &scale);

				for (const godot::Vector3i& pos : scale.create_commands)
				{
					// Try and create the node
					auto&& [it, emplaced] = scale.nodes.try_emplace(pos, spatial_world.builder.node_create());

					SpatialNode3D& node = *it->second;

					DEBUG_THREAD_CHECK_WRITE(&world, &node);

					// Initialize the node
					node.coord = SpatialCoord3D(pos, scale_index);
					node.last_update_time = world_time.frame_start;
					node.initialized = false;

					InitializeSpatialNode(node, spatial_world, scale_index);
				}

				scale.waiting_load_commands.insert(scale.waiting_load_commands.end(), scale.create_commands.begin(), scale.create_commands.end());
				scale.create_commands.clear();
			}
		});

		// System to move waiting load commands to be load commands
		world.system<const SpatialScale3DWorkerComponent, SpatialWorld3DComponent>(DEBUG_ONLY("WorldPrimeLoadCommands"))
			.multi_threaded()
			.kind<WorldLoadPhase>()
			.term_at(1).parent()
			.each([&world](const SpatialScale3DWorkerComponent& scale_worker, SpatialWorld3DComponent& spatial_world)
		{
			DEBUG_ASSERT(spatial_world.max_scale > 0, "The spatial world should have at least one scale");

			DEBUG_THREAD_CHECK_READ(&world, &spatial_world);

			size_t scale_index = scale_worker.scale;
			SpatialScale3D& scale = *spatial_world.scales[scale_index];

			DEBUG_THREAD_CHECK_WRITE(&world, &scale);

			// Clear any previous load commands since they should have been completed
			scale.load_commands.clear();

			size_t command_count = std::min(scale.waiting_load_commands.size(), k_max_frame_load_commands);

			if (command_count > 0)
			{
				scale.load_commands.assign(scale.waiting_load_commands.begin(), scale.waiting_load_commands.begin() + command_count);

				scale.waiting_load_commands.erase(scale.waiting_load_commands.begin(), scale.waiting_load_commands.begin() + command_count);
			}
		});

		// System to load spatial nodes that have been added
		world.system<const SpatialScale3DWorkerComponent, SpatialWorld3DComponent>(DEBUG_ONLY("WorldProcessLoadCommands"))
			.multi_threaded()
			.kind<WorldLoadPhase>()
			.term_at(1).parent()
			.each([&world](flecs::entity worker_entity, const SpatialScale3DWorkerComponent& scale_worker, SpatialWorld3DComponent& spatial_world)
		{
			DEBUG_THREAD_CHECK_READ(&world, &spatial_world);

			size_t scale_index = scale_worker.scale;
			SpatialScale3D& scale = *spatial_world.scales[scale_index];

			DEBUG_THREAD_CHECK_WRITE(&world, &scale);

			if (spatial_world.load_command_processors.empty()) // Don't continue if there aren't any processors but make sure to clear the commands
			{
				return;
			}

			RunEntityCommandsWithProcessors(worker_entity.parent(), spatial_world.load_command_processors, scale.load_commands, 
				[&world, &spatial_world, &scale](godot::Vector3i pos, auto& run_processors_delegate)
			{
				SpatialNodeMap::iterator it = scale.nodes.find(pos);
				DEBUG_ASSERT(it != scale.nodes.end(), "The node should have been initialized");
				DEBUG_ASSERT(it->second, "The node should have been initialized with data");

				SpatialNode3D& node = *it->second;

				DEBUG_THREAD_CHECK_WRITE(&world, &node);

				run_processors_delegate(spatial_world, scale, node);
			});
		});

		// System to move waiting load commands to be load commands
		world.system<const SpatialScale3DWorkerComponent, SpatialWorld3DComponent>(DEBUG_ONLY("WorldPrimeUnloadCommands"))
			.multi_threaded()
			.kind<WorldUnloadPhase>()
			.term_at(1).parent()
			.each([&world](const SpatialScale3DWorkerComponent& scale_worker, SpatialWorld3DComponent& spatial_world)
		{
			DEBUG_ASSERT(spatial_world.max_scale > 0, "The spatial world should have at least one scale");

			DEBUG_THREAD_CHECK_READ(&world, &spatial_world);

			size_t scale_index = scale_worker.scale;
			SpatialScale3D& scale = *spatial_world.scales[scale_index];

			DEBUG_THREAD_CHECK_WRITE(&world, &scale);

			// Clear any previous load commands since they should have been completed
			scale.unload_commands.clear();

			size_t command_count = std::min(scale.waiting_unload_commands.size(), k_max_frame_load_commands);

			if (command_count > 0)
			{
				scale.unload_commands.assign(scale.waiting_unload_commands.begin(), scale.waiting_unload_commands.begin() + command_count);

				scale.waiting_unload_commands.erase(scale.waiting_unload_commands.begin(), scale.waiting_unload_commands.begin() + command_count);
			}
		});

		// System to unload spatial nodes that have been marked to unload
		world.system<const SpatialScale3DWorkerComponent, SpatialWorld3DComponent>(DEBUG_ONLY("WorldProcessUnloadNodeCommands"))
			.multi_threaded()
			.kind<WorldUnloadPhase>()
			.term_at(1).parent()
			.each([&world](flecs::entity worker_entity, const SpatialScale3DWorkerComponent& scale_worker, SpatialWorld3DComponent& spatial_world)
		{
			DEBUG_THREAD_CHECK_READ(&world, &spatial_world);

			size_t scale_index = scale_worker.scale;
			SpatialScale3D& scale = *spatial_world.scales[scale_index];

			DEBUG_THREAD_CHECK_WRITE(&world, &scale);

			DEBUG_ASSERT(scale.destroy_commands.empty(), "We shouldn't have any destroy commands left over");

			if (spatial_world.unload_command_processors.empty()) // Don't continue if there aren't any processors but make sure to clear the commands
			{
				scale.destroy_commands = std::move(scale.unload_commands);
				return;
			}

			RunEntityCommandsWithProcessors(worker_entity.parent(), spatial_world.unload_command_processors, scale.unload_commands,
				[&world, &spatial_world, &scale](godot::Vector3i pos, auto& run_processors_delegate)
			{
				SpatialNodeMap::iterator it = scale.nodes.find(pos);
				DEBUG_ASSERT(it != scale.nodes.end(), "We should have only sent unload commands for existing nodes");

				SpatialNode3D& node = *it->second;

				DEBUG_THREAD_CHECK_WRITE(&world, &node);

				run_processors_delegate(spatial_world, scale, node);
			});

			scale.destroy_commands = std::move(scale.unload_commands);
		});

		// System to delete spatial nodes that have been marked to unload
		world.system<SpatialWorld3DComponent>(DEBUG_ONLY("WorldDestroyNodes"))
			.multi_threaded()
			.kind<WorldDestroyPhase>()
			.each([&world](SpatialWorld3DComponent& spatial_world)
		{
			DEBUG_THREAD_CHECK_WRITE(&world, &spatial_world);

			for (size_t scale_index = 0; scale_index < spatial_world.max_scale - 1; scale_index++)
			{
				SpatialScale3D& scale = *spatial_world.scales[scale_index];

				DEBUG_THREAD_CHECK_WRITE(&world, &scale);

				for (const godot::Vector3i& pos : scale.destroy_commands)
				{
					SpatialNodeMap::iterator it = scale.nodes.find(pos);
					DEBUG_ASSERT(it != scale.nodes.end(), "We should have only sent unload commands for existing nodes");

					SpatialNode3D& node = *it->second;

					DEBUG_THREAD_CHECK_WRITE(&world, &node);

					UninitializeSpatialNode(node, spatial_world, scale_index);

					spatial_world.builder.node_destroy(it->second);

					scale.nodes.erase(it);
				}

				scale.destroy_commands.clear();
			}
		});
	}

	void SpatialModule::AddSpatialScaleWorkers(flecs::world_t* world, flecs::entity_t spatial_world_entity)
	{
		flecs::scoped_world scope(world, spatial_world_entity);

		const SpatialWorld3DComponent* spatial_world = flecs::entity(world, spatial_world_entity).get<SpatialWorld3DComponent>();

		DEBUG_ASSERT(spatial_world != nullptr, "The entity should have a spatial world to add spatial workers");

		for (uint8_t scale_index = 0; scale_index < spatial_world->max_scale; scale_index++)
		{
			scope.entity(DEBUG_ONLY(godot::vformat("WorkerEntity_%d", scale_index).utf8()))
				.set(SpatialScale3DWorkerComponent{ scale_index });
		}
	}

	void SpatialModule::RemoveSpatialScaleWorkers(flecs::world_t* world, flecs::entity_t spatial_world_entity)
	{
		flecs::scoped_world scope(world, spatial_world_entity);

		scope.query_builder<const SpatialScale3DWorkerComponent>()
			.read(flecs::ChildOf, spatial_world_entity)
			.each([](flecs::entity entity, const SpatialScale3DWorkerComponent& scale_worker)
		{
			entity.destruct();
		});
	}

	SpatialNode3D* SpatialModule::GetNode(const SpatialWorld3DComponent& world, SpatialCoord3D coord)
	{
		DEBUG_ASSERT(coord.scale < k_max_world_scale, "The coordinates scale is out of range");

		SpatialNodeMap& nodes = world.scales[coord.scale]->nodes;

		SpatialNodeMap::iterator it = nodes.find(coord.pos);

		if (it == nodes.end())
		{
			return nullptr;
		}

		return it->second.get();
	}
}