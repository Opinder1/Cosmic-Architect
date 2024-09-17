#include "SpatialModule.h"
#include "SpatialComponents.h"
#include "SpatialTraverse.h"
#include "SpatialCommands.h"

#include "Simulation/SimulationComponents.h"

#include "Physics3D/PhysicsComponents.h"

#include "Util/Debug.h"
#include "Util/Callback.h"

namespace voxel_game::spatial3d
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

	void InitializeNode(Node& node, World& spatial_world, uint8_t scale_index)
	{
		Scale& scale = *spatial_world.scales[scale_index];

		for (uint8_t neighbour_index = 0; neighbour_index < 6; neighbour_index++)
		{
			godot::Vector3i neighbour_pos = node.coord.pos + node_neighbour_offsets[neighbour_index];

			NodeMap::iterator it = scale.nodes.find(neighbour_pos);

			if (it == scale.nodes.end())
			{
				continue;
			}

			Node* neighbour_node = it->second.get();

			node.neighbours[neighbour_index] = neighbour_node;
			node.neighbour_mask |= 1 << neighbour_index;

			neighbour_node->neighbours[5 - neighbour_index] = &node;
			neighbour_node->neighbour_mask |= 1 << (5 - neighbour_index);
		}

		if (scale_index < spatial_world.max_scale - 1)
		{
			Scale& parent_scale = *spatial_world.scales[scale_index + 1];

			Coord parent_pos = node.coord.GetParent();

			NodeMap::iterator it = parent_scale.nodes.find(parent_pos.pos);

			if (it != parent_scale.nodes.end())
			{
				Node& parent_node = *it->second;

				node.parent = &parent_node;
				node.parent_index = GetNodeParentIndex(node.coord.GetParentRelPos());

				DEBUG_ASSERT(node.parent_index < 8, "The parent index is out of range");

				parent_node.children_array[node.parent_index] = &node;
				parent_node.children_mask |= 1 << node.parent_index;
			}
		}

		if (scale_index > 0)
		{
			Scale& child_scale = *spatial_world.scales[scale_index - 1];

			for (uint8_t child_index = 0; child_index < 8; child_index++)
			{
				Coord child_pos = node.coord.GetBottomLeftChild();
				child_pos.pos += node_child_offsets[child_index];

				NodeMap::iterator it = child_scale.nodes.find(child_pos.pos);

				if (it != child_scale.nodes.end())
				{
					Node& child_node = *it->second;

					child_node.parent = &node;
					child_node.parent_index = child_index;

					node.children_array[child_index] = &child_node;
					node.children_mask |= 1 << child_index;
				}
			}
		}
	}

	void UninitializeNode(Node& node, World& spatial_world, uint8_t scale_index)
	{
		for (uint8_t neighbour_index = 0; neighbour_index < 6; neighbour_index++)
		{
			if (Node* neighbour_node = node.neighbours[neighbour_index])
			{
				neighbour_node->neighbours[5 - neighbour_index] = nullptr;
				neighbour_node->neighbour_mask &= ~(1 << (5 - neighbour_index));
			}
		}

		if (scale_index < spatial_world.max_scale - 1)
		{
			if (Node* parent_node = node.parent)
			{
				parent_node->children_array[node.parent_index] = nullptr;
				parent_node->children_mask &= ~(1 << node.parent_index);
			}
		}

		if (scale_index > 0)
		{
			for (uint8_t child_index = 0; child_index < 8; child_index++)
			{
				if (Node* child_node = node.children_array[child_index])
				{
					child_node->parent = nullptr;
					child_node->parent_index = k_node_no_parent;
				}
			}
		}
	}

	Module::Module(flecs::world& world)
	{
		world.module<Module>();

		world.import<Components>();
		world.import<sim::Components>();
		world.import<physics3d::Components>();

		// Relationships

		world.singleton<Entity>()
			.add_second<World>(flecs::OneOf)
			.add_second<physics3d::Position>(flecs::With);

		world.singleton<ScaleWorker>()
			.add_second<World>(flecs::OneOf);

		world.singleton<RegionWorker>()
			.add_second<World>(flecs::OneOf);

		world.singleton<Loader>()
			.add_second<Entity>(flecs::With);

		// Phases

		// The phase that all spatial wrold processing happens in
		world.singleton<WorldMultithreadPhase>()
			.add(flecs::Phase)
			.depends_on(flecs::OnUpdate);

		CreateSyncedPhase<WorldRegionWorkerPhase, WorldMultithreadPhase>(world);
		CreateSyncedPhase<WorldScaleWorkerPhase, WorldRegionWorkerPhase>(world);
		CreateSyncedPhase<WorldWorkerPhase, WorldScaleWorkerPhase>(world);
		CreateSyncedPhase<WorldCreatePhase, WorldWorkerPhase>(world);
		CreateSyncedPhase<WorldLoadPhase, WorldCreatePhase>(world);
		CreateSyncedPhase<WorldUnloadPhase, WorldLoadPhase>(world);
		CreateSyncedPhase<WorldDestroyPhase, WorldUnloadPhase>(world);
		CreateSyncedPhase<WorldEndPhase, WorldDestroyPhase>(world);

		world.component<World>()
			.on_remove([](spatial3d::World& spatial_world)
		{
			if (!spatial_world.initialized)
			{
				DEBUG_PRINT_ERROR("The spatial world should have been initialized");
				return;
			}

			for (uint8_t i = 0; i < spatial_world.max_scale; i++)
			{
				spatial3d::ScalePtr& scale = spatial_world.scales[i];

				if (!scale)
				{
					continue;
				}

				DEBUG_ASSERT(scale->nodes.size() == 0, "We should have cleaned up all of our nodes before being finally destroyed");

				for (auto&& [pos, node] : scale->nodes)
				{
					spatial_world.builder.node_destroy(node);
				}

				spatial_world.builder.scale_destroy(scale);
			}
		});

		// Observers

		// Add a cached query for all of a spatial worlds child nodes for fast access
		world.observer<World>(DEBUG_ONLY("AddWorldQueries"))
			.event(flecs::OnAdd)
			.each([](flecs::entity world_entity, World& spatial_world)
		{
			flecs::scoped_world scope = world_entity.scope(); // Add the queries as children of the entity so they are automatically destructed

			spatial_world.entities_query = scope.query_builder<const Entity>(DEBUG_ONLY("WorldEntitiesQuery"))
				.with(flecs::ChildOf, world_entity).read() // Use read() as its required for queries run inside systems
				.cached()
				.build().c_ptr();

			spatial_world.scale_workers_query = scope.query_builder<const ScaleWorker>(DEBUG_ONLY("WorldScaleWorkersQuery"))
				.with(flecs::ChildOf, world_entity).read() // Use read() as its required for queries run inside systems
				.cached()
				.build().c_ptr();

			spatial_world.region_workers_query = scope.query_builder<const RegionWorker>(DEBUG_ONLY("WorldRegionWorkersQuery"))
				.with(flecs::ChildOf, world_entity).read() // Use read() as its required for queries run inside systems
				.cached()
				.build().c_ptr();

			spatial_world.loaders_query = scope.query_builder<const Loader, const physics3d::Position>(DEBUG_ONLY("WorldLoadersQuery"))
				.with(flecs::ChildOf, world_entity).read() // Use read() as its required for queries run inside systems
				.cached()
				.build().c_ptr();
		});

		// Systems

		// System to mark any nodes that are no longer being observed to be unloaded
		world.system<const ScaleWorker, World, const sim::GlobalTime>(DEBUG_ONLY("ScaleUnloadUnusedNodes"))
			.multi_threaded()
			.interval(0.25)
			.kind<WorldScaleWorkerPhase>()
			.term_at(1).parent()
			.term_at(2).src<sim::GlobalTime>()
			.each([&world](const ScaleWorker& scale_worker, World& spatial_world, const sim::GlobalTime& world_time)
		{
			DEBUG_THREAD_CHECK_READ(&world, &spatial_world);
			DEBUG_THREAD_CHECK_READ(&world, &world_time);

			size_t scale_index = scale_worker.scale;
			Scale& scale = *spatial_world.scales[scale_index];

			DEBUG_THREAD_CHECK_WRITE(&world, &scale);

			for (auto&& [pos, node] : scale.nodes)
			{
				DEBUG_THREAD_CHECK_WRITE(&world, &node);

				if (world_time.frame_start - node->last_update_time < 10s)
				{
					continue;
				}

				if (node->state == NodeState::Unloaded)
				{
					node->state = NodeState::Deleting;
				}
				else if (node->state == NodeState::Loaded)
				{
					if (scale.unload_commands.size() < k_max_frame_unload_commands)
					{
						scale.unload_commands.push_back(node->coord.pos);
						node->state = NodeState::Unloading;
					}
				}
			}
		});

		// Systen to create or update all nodes in the range of loaders
		world.system<const ScaleWorker, World, const sim::GlobalTime>(DEBUG_ONLY("LoaderCreateNodes"))
			.multi_threaded()
			.interval(0.25)
			.kind<WorldScaleWorkerPhase>()
			.term_at(1).parent()
			.term_at(2).src<sim::GlobalTime>()
			.each([&world](flecs::entity worker_entity, const ScaleWorker& scale_worker, World& spatial_world, const sim::GlobalTime& world_time)
		{
			DEBUG_THREAD_CHECK_READ(&world, &spatial_world);
			DEBUG_THREAD_CHECK_READ(&world, &world_time);

			size_t scale_index = scale_worker.scale;
			Scale& scale = *spatial_world.scales[scale_index];

			DEBUG_THREAD_CHECK_WRITE(&world, &scale);

			flecs::query<const Loader, const physics3d::Position> staged_loaders_query(spatial_world.loaders_query);

			// For each command list that is a child of the world
			staged_loaders_query.iter(worker_entity.world())
				.each([&world, scale_index, &scale, &spatial_world, &world_time](const Loader& spatial_loader, const physics3d::Position& position)
			{
				const uint32_t scale_step = 1 << scale_index;
				const uint32_t scale_node_step = scale_step * spatial_world.node_size;

				DEBUG_THREAD_CHECK_READ(&world, &spatial_loader);

				if (scale_index < spatial_loader.min_lod || scale_index > spatial_loader.max_lod)
				{
					return;
				}

				ForEachCoordInSphere(position.position / scale_node_step, spatial_loader.dist_per_lod, [&world, &scale, &world_time](godot::Vector3i pos)
				{
					NodeMap::iterator it = scale.nodes.find(pos);

					if (it != scale.nodes.end())
					{
						Node& node = *it->second;

						DEBUG_THREAD_CHECK_WRITE(&world, &node);

						node.last_update_time = world_time.frame_start;

						if (node.state == NodeState::Unloaded)
						{
							if (scale.load_commands.size() < k_max_frame_load_commands)
							{
								scale.load_commands.push_back(node.coord.pos);
								node.state = NodeState::Loading;
							}
						}
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
		world.system<World, const sim::GlobalTime>(DEBUG_ONLY("WorldCreateNodes"))
			.multi_threaded()
			.kind<WorldCreatePhase>()
			.term_at(1).src<sim::GlobalTime>()
			.each([&world](World& spatial_world, const sim::GlobalTime& world_time)
		{
			DEBUG_ASSERT(spatial_world.max_scale > 0, "The spatial world should have at least one scale");

			DEBUG_THREAD_CHECK_WRITE(&world, &spatial_world);

			// Initialize the largest scales first
			for (size_t scale_index = spatial_world.max_scale; scale_index-- > 0;)
			{
				Scale& scale = *spatial_world.scales[scale_index];

				DEBUG_THREAD_CHECK_WRITE(&world, &scale);

				for (const godot::Vector3i& pos : scale.create_commands)
				{
					// Try and create the node
					auto&& [it, emplaced] = scale.nodes.try_emplace(pos, spatial_world.builder.node_create());

					DEBUG_ASSERT(emplaced, "The node should have been emplaced");

					Node& node = *it->second;

					DEBUG_THREAD_CHECK_WRITE(&world, &node);

					// Initialize the node
					node.coord = Coord(pos, scale_index);
					node.last_update_time = world_time.frame_start;
					node.state = NodeState::Unloaded;

					InitializeNode(node, spatial_world, scale_index);
				}

				scale.create_commands.clear();
			}
		});

		// System to load spatial nodes that have been added
		world.system<const ScaleWorker, World>(DEBUG_ONLY("WorldProcessLoadCommands"))
			.multi_threaded()
			.kind<WorldLoadPhase>()
			.term_at(1).parent()
			.each([&world](flecs::entity worker_entity, const ScaleWorker& scale_worker, World& spatial_world)
		{
			DEBUG_THREAD_CHECK_READ(&world, &spatial_world);

			size_t scale_index = scale_worker.scale;
			Scale& scale = *spatial_world.scales[scale_index];

			DEBUG_THREAD_CHECK_WRITE(&world, &scale);

			if (spatial_world.load_command_processors.empty()) // Don't continue if there aren't any processors but make sure to clear the commands
			{
				scale.load_commands.clear();
				return;
			}

			if (scale.load_commands.empty())
			{
				return;
			}

			auto node_callback = [&world, &spatial_world, &scale](godot::Vector3i pos, auto& node_processors_callback)
			{
				NodeMap::iterator it = scale.nodes.find(pos);
				DEBUG_ASSERT(it != scale.nodes.end(), "The node should have been initialized");
				DEBUG_ASSERT(it->second, "The node should have been initialized with data");

				Node& node = *it->second;

				DEBUG_THREAD_CHECK_WRITE(&world, &node);

				DEBUG_ASSERT(node.state == NodeState::Loading, "Node should be in loading state");

				node_processors_callback(spatial_world, scale, node);

				node.state = NodeState::Loaded;
			};

			EntityCommandExecutor node_command_executor(worker_entity.parent(), spatial_world.load_command_processors.data(), spatial_world.load_command_processors.size());

			node_command_executor.Run(scale.load_commands.begin(), scale.load_commands.end(), node_callback);

			scale.load_commands.clear();
		});

		// System to unload spatial nodes that have been marked to unload
		world.system<const ScaleWorker, World>(DEBUG_ONLY("WorldProcessUnloadNodeCommands"))
			.multi_threaded()
			.kind<WorldUnloadPhase>()
			.term_at(1).parent()
			.each([&world](flecs::entity worker_entity, const ScaleWorker& scale_worker, World& spatial_world)
		{
			DEBUG_THREAD_CHECK_READ(&world, &spatial_world);

			size_t scale_index = scale_worker.scale;
			Scale& scale = *spatial_world.scales[scale_index];

			DEBUG_THREAD_CHECK_WRITE(&world, &scale);

			DEBUG_ASSERT(scale.destroy_commands.empty(), "We shouldn't have any destroy commands left over");

			if (spatial_world.unload_command_processors.empty()) // Don't continue if there aren't any processors but make sure to clear the commands
			{
				scale.unload_commands.clear();
				return;
			}

			if (scale.unload_commands.empty())
			{
				return;
			}

			auto node_callback = [&world, &spatial_world, &scale](godot::Vector3i pos, auto& node_processors_callback)
			{
				NodeMap::iterator it = scale.nodes.find(pos);
				DEBUG_ASSERT(it != scale.nodes.end(), "We should have only sent unload commands for existing nodes");

				Node& node = *it->second;

				DEBUG_THREAD_CHECK_WRITE(&world, &node);

				DEBUG_ASSERT(node.state == NodeState::Unloading, "Node should be in unloading state");

				node_processors_callback(spatial_world, scale, node);

				node.state = NodeState::Unloaded;
			};

			EntityCommandExecutor node_command_executor(worker_entity.parent(), spatial_world.unload_command_processors.data(), spatial_world.unload_command_processors.size());

			node_command_executor.Run(scale.unload_commands.begin(), scale.unload_commands.end(), node_callback);

			scale.unload_commands.clear();
		});

		// System to delete spatial nodes that have been marked to unload
		world.system<World>(DEBUG_ONLY("WorldDestroyNodes"))
			.multi_threaded()
			.kind<WorldDestroyPhase>()
			.each([&world](World& spatial_world)
		{
			DEBUG_THREAD_CHECK_WRITE(&world, &spatial_world);

			for (size_t scale_index = 0; scale_index < spatial_world.max_scale; scale_index++)
			{
				Scale& scale = *spatial_world.scales[scale_index];

				DEBUG_THREAD_CHECK_WRITE(&world, &scale);

				for (const godot::Vector3i& pos : scale.destroy_commands)
				{
					NodeMap::iterator it = scale.nodes.find(pos);
					DEBUG_ASSERT(it != scale.nodes.end(), "We should have only sent unload commands for existing nodes");

					Node& node = *it->second;

					DEBUG_THREAD_CHECK_WRITE(&world, &node);

					DEBUG_ASSERT(node.state == NodeState::Deleting, "Node should be in deleting state");

					UninitializeNode(node, spatial_world, scale_index);

					spatial_world.builder.node_destroy(it->second);

					it = scale.nodes.erase(it);

					DEBUG_ASSERT(it != scale.nodes.end(), "The node should have been erased");
				}

				scale.destroy_commands.clear();
			}
		});
	}

	void AddScaleWorkers(flecs::entity spatial_world_entity)
	{
		flecs::scoped_world scope = spatial_world_entity.scope();

		const World* spatial_world = spatial_world_entity.get<World>();

		DEBUG_ASSERT(spatial_world != nullptr, "The entity should have a spatial world to add spatial workers");

		for (uint8_t scale_index = 0; scale_index < spatial_world->max_scale; scale_index++)
		{
			godot::String worker_name = godot::vformat("WorkerEntity%d", scale_index);

			flecs::entity worker_entity(scope, DEBUG_ONLY(worker_name.utf8()));

			worker_entity.set(ScaleWorker{ scale_index });
		}
	}

	void RemoveScaleWorkers(flecs::entity spatial_world_entity)
	{
		flecs::scoped_world scope = spatial_world_entity.scope();

		scope.query_builder<const ScaleWorker>()
			.read(flecs::ChildOf, spatial_world_entity)
			.each([](flecs::entity entity, const ScaleWorker& scale_worker)
		{
			entity.destruct();
		});
	}

	Node* GetNode(const World& world, Coord coord)
	{
		DEBUG_ASSERT(coord.scale < k_max_world_scale, "The coordinates scale is out of range");

		NodeMap& nodes = world.scales[coord.scale]->nodes;

		NodeMap::iterator it = nodes.find(coord.pos);

		if (it == nodes.end())
		{
			return nullptr;
		}

		return it->second.get();
	}
}