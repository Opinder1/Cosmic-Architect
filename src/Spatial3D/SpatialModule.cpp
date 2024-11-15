#include "SpatialModule.h"
#include "SpatialComponents.h"
#include "SpatialTraverse.h"

#include "Simulation/SimulationComponents.h"

#include "Physics3D/PhysicsComponents.h"

#include "Util/Debug.h"
#include "Util/Callback.h"

#include <easy/profiler.h>

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

	flecs::entity CreateSyncPhase(flecs::world& world, flecs::entity_t parent)
	{
		flecs::entity sync_phase = world.entity(DEBUG_ONLY("ThreadSyncPhase"))
			.add(flecs::Phase)
			.child_of(parent)
			.depends_on(parent);

		world.system(DEBUG_ONLY("ThreadSyncSystem"))
			.immediate()
			.kind(sync_phase)
			.run([](flecs::iter& it) {});

		return sync_phase;
	}

	void InitializeNode(Node& node, World& spatial_world, uint8_t scale_index)
	{
		Scale& scale = spatial_world.scales[scale_index];

		for (uint8_t neighbour_index = 0; neighbour_index < 6; neighbour_index++)
		{
			godot::Vector3i neighbour_pos = node.coord.pos + node_neighbour_offsets[neighbour_index];

			NodeMap::iterator it = scale.nodes.find(neighbour_pos);

			if (it != scale.nodes.end())
			{
				Node& neighbour_node = it->second.GetEntry(spatial_world.node_entry);

				node.neighbours[neighbour_index] = &neighbour_node;
				node.neighbour_mask |= 1 << neighbour_index;

				neighbour_node.neighbours[5 - neighbour_index] = &node;
				neighbour_node.neighbour_mask |= 1 << (5 - neighbour_index);
			}
		}

		if (scale_index < spatial_world.max_scale - 1)
		{
			Scale& parent_scale = spatial_world.scales[scale_index + 1];

			Coord parent_pos = node.coord.GetParent();

			NodeMap::iterator it = parent_scale.nodes.find(parent_pos.pos);

			if (it != parent_scale.nodes.end())
			{
				Node& parent_node = it->second.GetEntry(spatial_world.node_entry);

				node.parent = &parent_node;
				node.parent_index = GetNodeParentIndex(node.coord.GetParentRelPos());

				DEBUG_ASSERT(node.parent_index < 8, "The parent index is out of range");

				parent_node.children_array[node.parent_index] = &node;
				parent_node.children_mask |= 1 << node.parent_index;
			}
		}

		if (scale_index > 0)
		{
			Scale& child_scale = spatial_world.scales[scale_index - 1];

			for (uint8_t child_index = 0; child_index < 8; child_index++)
			{
				Coord child_pos = node.coord.GetBottomLeftChild();
				child_pos.pos += node_child_offsets[child_index];

				NodeMap::iterator it = child_scale.nodes.find(child_pos.pos);

				if (it != child_scale.nodes.end())
				{
					Node& child_node = it->second.GetEntry(spatial_world.node_entry);

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

		world.singleton<ScaleMarker>()
			.add_second<World>(flecs::OneOf);

		world.singleton<RegionMarker>()
			.add_second<World>(flecs::OneOf);

		world.singleton<Loader>()
			.add_second<Entity>(flecs::With);

		// Phases

		world.singleton<NodeCreatePhase>()
			.add(flecs::Phase)
			.depends_on(flecs::OnUpdate);

		world.singleton<NodeLoadPhase>()
			.add(flecs::Phase)
			.depends_on(flecs::OnUpdate);

		world.singleton<NodeUnloadPhase>()
			.add(flecs::Phase)
			.depends_on(flecs::OnUpdate);

		world.singleton<NodeDestroyPhase>()
			.add(flecs::Phase)
			.depends_on(flecs::OnUpdate);

		// Observers

		// Add a cached query for all of a spatial worlds child nodes for fast access
		world.observer<World>(DEBUG_ONLY("AddWorldQueries"))
			.event(flecs::OnAdd)
			.each([](flecs::entity world_entity, World& spatial_world)
		{
			flecs::scoped_world scope = world_entity.scope(); // Add the queries as children of the entity so they are automatically destructed

			spatial_world.node_entry = spatial_world.node_type.AddEntry<Node>();

			spatial_world.entities_query = scope.query_builder<const Entity>(DEBUG_ONLY("WorldEntitiesQuery"))
				.with(flecs::ChildOf, world_entity)
				.cached()
				.build();

			spatial_world.loaders_query = scope.query_builder<const Loader, const physics3d::Position>(DEBUG_ONLY("WorldLoadersQuery"))
				.with(flecs::ChildOf, world_entity)
				.cached()
				.build();

			spatial_world.scale_marker_query = scope.query_builder<const ScaleMarker>(DEBUG_ONLY("WorldScaleMarkerQuery"))
				.with(flecs::ChildOf, world_entity)
				.cached()
				.build();

			spatial_world.region_marker_query = scope.query_builder<const RegionMarker>(DEBUG_ONLY("WorldRegionMarkerQuery"))
				.with(flecs::ChildOf, world_entity)
				.cached()
				.build();
		});

		// Systems

		// System to mark any nodes that are no longer being observed to be unloaded
		world.system<World, const sim::GlobalTime>(DEBUG_ONLY("ScaleUnloadUnusedNodes"))
			.multi_threaded()
			.term_at(1).src<sim::GlobalTime>()
			.each([](World& spatial_world, const sim::GlobalTime& world_time)
		{
			EASY_BLOCK("ScaleUnloadUnusedNodes");

			for (size_t scale_index = 0; scale_index < spatial_world.max_scale; scale_index++)
			{
				Scale& scale = spatial_world.scales[scale_index];

				// Clear previous commands. If you didn't handle them then too bad
				scale.unload_commands.clear();
				scale.destroy_commands.clear();

				// For each node in the scale
				for (auto&& [pos, node_poly] : scale.nodes)
				{
					Node& node = node_poly.GetEntry(spatial_world.node_entry);

					// Check if node hasn't been touched in too long
					if (world_time.frame_start - node.last_update_time < spatial_world.node_keepalive)
					{
						continue;
					}

					// Move the entity along to deletion
					switch (node.state)
					{
					case NodeState::Unloaded:
						if (scale.destroy_commands.size() < k_max_frame_destroy_commands)
						{
							scale.destroy_commands.push_back(node_poly);
							node.state = NodeState::Deleting;
						}
						break;

					case NodeState::Loaded:
						if (scale.unload_commands.size() < k_max_frame_unload_commands)
						{
							scale.unload_commands.push_back(node_poly);
							node.state = NodeState::Unloading;
						}
						break;
					}
				}
			}
		});

		// Systen to create or update all nodes in the range of loaders
		world.system<World, const sim::GlobalTime>(DEBUG_ONLY("LoaderTouchNodes"))
			.multi_threaded()
			.term_at(1).src<sim::GlobalTime>()
			.each([world](World& spatial_world, const sim::GlobalTime& world_time)
		{
			EASY_BLOCK("LoaderTouchNodes");

			for (size_t scale_index = 0; scale_index < spatial_world.max_scale; scale_index++)
			{
				Scale& scale = spatial_world.scales[scale_index];

				// Clear previous commands. If you didn't handle them then too bad
				scale.create_commands.clear();
				scale.load_commands.clear();

				// For each command list that is a child of the world
				spatial_world.loaders_query.iter(world).each([&](const Loader& spatial_loader, const physics3d::Position& position)
				{
					EASY_BLOCK("SingleLoader");

					const uint32_t scale_step = 1 << scale_index;
					const double scale_node_step = scale_step * spatial_world.node_size;

					if (scale_index < spatial_loader.min_lod || scale_index > spatial_loader.max_lod)
					{
						return;
					}

					// For each node in the sphere of the loader
					ForEachCoordInSphere(position.position / scale_node_step, spatial_loader.dist_per_lod, [&](godot::Vector3i pos)
					{
						NodeMap::iterator it = scale.nodes.find(pos);

						if (it == scale.nodes.end())
						{
							scale.create_commands.push_back(pos); // Create commands should immediately be executed this frame so don't worry about duplicates
							return;
						}

						Node& node = it->second.GetEntry(spatial_world.node_entry);

						// Touch the node so it stays loaded
						node.last_update_time = world_time.frame_start;

						// Load the node if its not loaded yet
						switch (node.state)
						{
						case NodeState::Unloaded:
							if (scale.load_commands.size() < k_max_frame_load_commands)
							{
								scale.load_commands.push_back(it->second);
								node.state = NodeState::Loading;
							}
							break;
						}
					});
				});
			}
		});

		// System to initialize spatial nodes that have been added
		world.system<World, const sim::GlobalTime>(DEBUG_ONLY("WorldCreateNodes"))
			.multi_threaded()
			.term_at(1).src<sim::GlobalTime>()
			.each([](World& spatial_world, const sim::GlobalTime& world_time)
		{
			EASY_BLOCK("WorldCreateNodes");

			DEBUG_ASSERT(spatial_world.max_scale > 0, "The spatial world should have at least one scale");

			for (size_t scale_index = 0; scale_index < spatial_world.max_scale; scale_index++)
			{
				EASY_BLOCK("ScaleCreateNodes");

				Scale& scale = spatial_world.scales[scale_index];

				// For each create command
				for (const godot::Vector3i& pos : scale.create_commands)
				{
					// Try and create the node
					auto&& [it, emplaced] = scale.nodes.try_emplace(pos, spatial_world.node_type.CreatePoly());

					DEBUG_ASSERT(emplaced, "The node should have been emplaced. We must have had a duplicate command.");

					Node& node = it->second.GetEntry(spatial_world.node_entry);

					// Initialize the node
					node.coord = Coord(pos, scale_index);
					node.last_update_time = world_time.frame_start;
					node.state = NodeState::Unloaded;

					InitializeNode(node, spatial_world, scale_index);
				}
			}
		});

		// System to delete spatial nodes that have been marked to unload
		world.system<World>(DEBUG_ONLY("WorldDestroyNodes"))
			.multi_threaded()
			.each([](World& spatial_world)
		{
			EASY_BLOCK("WorldDestroyNodes");

			for (size_t scale_index = 0; scale_index < spatial_world.max_scale; scale_index++)
			{
				EASY_BLOCK("ScaleDestroyNodes");

				Scale& scale = spatial_world.scales[scale_index];

				// For each destroy command of the scale
				for (Poly node_poly : scale.destroy_commands)
				{
					Node& node = node_poly.GetEntry(spatial_world.node_entry);

					DEBUG_ASSERT(node.state == NodeState::Deleting, "Node should be in deleting state");

					UninitializeNode(node, spatial_world, scale_index);

					spatial_world.node_type.DestroyPoly(node_poly);

					scale.nodes.erase(node.coord.pos);
				}
			}
		});
	}

	void AddScaleMarkers(flecs::entity spatial_world_entity)
	{
		flecs::scoped_world scope = spatial_world_entity.scope();

		const World* spatial_world = spatial_world_entity.get<World>();

		DEBUG_ASSERT(spatial_world != nullptr, "The entity should have a spatial world to add spatial workers");

		for (uint8_t scale_index = 0; scale_index < spatial_world->max_scale; scale_index++)
		{
			godot::String worker_name = godot::vformat("WorkerEntity%d", scale_index);

			flecs::entity worker_entity(scope, DEBUG_ONLY(worker_name.utf8()));

			worker_entity.set(ScaleMarker{ scale_index });
		}
	}

	void RemoveScaleMarkers(flecs::entity spatial_world_entity)
	{
		flecs::scoped_world scope = spatial_world_entity.scope();

		scope.query_builder<const ScaleMarker>()
			.read(flecs::ChildOf, spatial_world_entity)
			.each([](flecs::entity entity, const ScaleMarker& scale_marker)
		{
			entity.destruct();
		});
	}

	Poly GetNode(const World& world, Coord coord)
	{
		DEBUG_ASSERT(coord.scale < k_max_world_scale, "The coordinates scale is out of range");

		const Scale& scale = world.scales[coord.scale];

		NodeMap::const_iterator it = scale.nodes.find(coord.pos);

		if (it == scale.nodes.end())
		{
			return Poly{};
		}

		return it->second;
	}
}