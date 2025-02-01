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

	void InitializeNode(World& spatial_world, Poly node_poly, uint8_t scale_index)
	{
		Scale& scale = GetScale(spatial_world, scale_index);

		Node& node = GetNode(spatial_world, node_poly);

		for (uint8_t neighbour_index = 0; neighbour_index < 6; neighbour_index++)
		{
			godot::Vector3i neighbour_pos = node.coord.pos + node_neighbour_offsets[neighbour_index];

			NodeMap::iterator it = scale.nodes.find(neighbour_pos);

			if (it != scale.nodes.end())
			{
				Poly neighbour_node_poly = it->second;
				Node& neighbour_node = GetNode(spatial_world, neighbour_node_poly);

				node.neighbours[neighbour_index] = neighbour_node_poly;
				node.neighbour_mask |= 1 << neighbour_index;

				neighbour_node.neighbours[5 - neighbour_index] = node_poly;
				neighbour_node.neighbour_mask |= 1 << (5 - neighbour_index);
			}
		}

		if (scale_index < spatial_world.max_scale - 1)
		{
			Scale& parent_scale = GetScale(spatial_world, scale_index + 1);

			Coord parent_pos = node.coord.GetParent();

			NodeMap::iterator it = parent_scale.nodes.find(parent_pos.pos);

			if (it != parent_scale.nodes.end())
			{
				Poly parent_node_poly = it->second;
				Node& parent_node = GetNode(spatial_world, parent_node_poly);

				node.parent = parent_node_poly;
				node.parent_index = GetNodeParentIndex(node.coord.GetParentRelPos());

				DEBUG_ASSERT(node.parent_index < 8, "The parent index is out of range");

				parent_node.children_array[node.parent_index] = node_poly;
				parent_node.children_mask |= 1 << node.parent_index;
			}
		}

		if (scale_index > 0)
		{
			Scale& child_scale = GetScale(spatial_world, scale_index - 1);

			for (uint8_t child_index = 0; child_index < 8; child_index++)
			{
				Coord child_pos = node.coord.GetBottomLeftChild();
				child_pos.pos += node_child_offsets[child_index];

				NodeMap::iterator it = child_scale.nodes.find(child_pos.pos);

				if (it != child_scale.nodes.end())
				{
					Poly child_node_poly = it->second;
					Node& child_node = GetNode(spatial_world, child_node_poly);

					child_node.parent = node_poly;
					child_node.parent_index = child_index;

					node.children_array[child_index] = child_node_poly;
					node.children_mask |= 1 << child_index;
				}
			}
		}
	}

	void UninitializeNode(World& spatial_world, Poly node_poly, uint8_t scale_index)
	{
		Node& node = GetNode(spatial_world, node_poly);

		for (uint8_t neighbour_index = 0; neighbour_index < 6; neighbour_index++)
		{
			Poly neighbour_node_poly = node.neighbours[neighbour_index];

			if (neighbour_node_poly.IsValid())
			{
				Node& neighbour_node = GetNode(spatial_world, neighbour_node_poly);

				neighbour_node.neighbours[5 - neighbour_index] = nullptr;
				neighbour_node.neighbour_mask &= ~(1 << (5 - neighbour_index));
			}
		}

		if (scale_index < spatial_world.max_scale - 1)
		{
			Poly parent_node_poly = node.parent;

			if (parent_node_poly.IsValid())
			{
				Node& parent_node = GetNode(spatial_world, parent_node_poly);

				parent_node.children_array[node.parent_index] = nullptr;
				parent_node.children_mask &= ~(1 << node.parent_index);
			}
		}

		if (scale_index > 0)
		{
			for (uint8_t child_index = 0; child_index < 8; child_index++)
			{
				Poly child_node_poly = node.children_array[child_index];

				if (child_node_poly.IsValid())
				{
					Node& child_node = GetNode(spatial_world, child_node_poly);

					child_node.parent = nullptr;
					child_node.parent_index = k_node_no_parent;
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

		world.singleton<RScale>()
			.add(flecs::Relationship);

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
		world.observer<WorldMarker>(DEBUG_ONLY("AddWorldQueries"))
			.event(flecs::OnAdd)
			.each([](flecs::entity world_entity, WorldMarker& spatial_world)
		{
			flecs::scoped_world world = world_entity.scope(); // Add the queries as children of the entity so they are automatically destructed

			spatial_world.world.node_entry = spatial_world.world.node_type.AddEntry<Node>();
			spatial_world.world.scale_entry = spatial_world.world.scale_type.AddEntry<Scale>();

			spatial_world.loaders_query = world.query_builder<const Loader, const physics3d::Position>(DEBUG_ONLY("WorldLoadersQuery"))
				.with(flecs::ChildOf, world_entity)
				.cached()
				.build();

			spatial_world.scale_marker_query = world.query_builder<const ScaleMarker>(DEBUG_ONLY("ScaleMarkerQuery"))
				.with(flecs::ChildOf, world_entity)
				.cached()
				.build();

			spatial_world.region_marker_query = world.query_builder<const RegionMarker>(DEBUG_ONLY("WorldRegionMarkerQuery"))
				.with(flecs::ChildOf, world_entity)
				.cached()
				.build();

			spatial_world.node_marker_query = world.query_builder<const NodeMarker>(DEBUG_ONLY("WorldNodeMarkerQuery"))
				.with(flecs::ChildOf, world_entity)
				.cached()
				.build();
		});

		// Add a cached query for all of a spatial worlds child nodes for fast access
		world.observer<ScaleMarker, WorldMarker>(DEBUG_ONLY("AddScaleQueries"))
			.event(flecs::OnSet)
			.term_at(1).up(flecs::ChildOf).filter()
			.each([](flecs::entity marker_entity, ScaleMarker& marker, WorldMarker& spatial_world)
		{
			flecs::entity world_entity = marker_entity.parent();

			flecs::scoped_world world = marker_entity.scope(); // Add the queries as children of the entity so they are automatically destructed

			Poly scale_poly = spatial_world.world.scales[marker.scale];
			Scale& scale = GetScale(spatial_world.world, scale_poly);

			scale.scale = marker.scale;

			marker.update_parent_node_query = world.query_builder<Entity, const physics3d::Position>(DEBUG_ONLY("MarkerEntitiesQuery"))
				.with(flecs::ChildOf, world_entity)
				.with<RScale>(marker_entity)
				.cached()
				.build();

			DEBUG_ASSERT(marker.update_parent_node_query, "Query should be valid");
		});

		world.observer<WorldMarker>()
			.event(flecs::OnRemove)
			.each([](WorldMarker& spatial_world)
		{
			for (Poly scale_poly : spatial_world.world.scales)
			{
				Scale& scale = GetScale(spatial_world.world, scale_poly);

				DEBUG_ASSERT(scale.create_commands.empty(), "All commands should have been destroyed before destroying the world");
				DEBUG_ASSERT(scale.load_commands.empty(), "All commands should have been destroyed before destroying the world");
				DEBUG_ASSERT(scale.unload_commands.empty(), "All commands should have been destroyed before destroying the world");
				DEBUG_ASSERT(scale.destroy_commands.empty(), "All commands should have been destroyed before destroying the world");
				DEBUG_ASSERT(scale.nodes.empty(), "All nodes should have been destroyed before destroying the world");
			}
		});

		// Systems

		// System to mark any nodes that are no longer being observed to be unloaded
		world.system<WorldMarker, const sim::GlobalTime>(DEBUG_ONLY("ScaleUnloadUnusedNodes"))
			.multi_threaded()
			.term_at(1).src<sim::GlobalTime>()
			.each([](flecs::entity worker_entity, WorldMarker& spatial_world, const sim::GlobalTime& world_time)
		{
			EASY_BLOCK("ScaleUnloadUnusedNodes");

			for (size_t scale_index = 0; scale_index < spatial_world.world.max_scale; scale_index++)
			{
				Scale& scale = GetScale(spatial_world.world, scale_index);

				// Clear previous commands. If you didn't handle them then too bad
				scale.unload_commands.clear();
				scale.destroy_commands.clear();

				// For each node in the scale
				for (auto&& [pos, node_poly] : scale.nodes)
				{
					Node& node = GetNode(spatial_world.world, node_poly);

					// Check if node hasn't been touched in too long
					if (world_time.frame_start - node.last_update_time < spatial_world.world.node_keepalive)
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
		world.system<WorldMarker, const sim::GlobalTime>(DEBUG_ONLY("LoaderTouchNodes"))
			.multi_threaded()
			.term_at(1).src<sim::GlobalTime>()
			.each([](flecs::entity worker_entity, WorldMarker& spatial_world, const sim::GlobalTime& world_time)
		{
			EASY_BLOCK("LoaderTouchNodes");

			for (size_t scale_index = 0; scale_index < spatial_world.world.max_scale; scale_index++)
			{
				Scale& scale = GetScale(spatial_world.world, scale_index);

				// Finish the previous load commands
				for (Poly node_poly : scale.load_commands)
				{
					Node& node = GetNode(spatial_world.world, node_poly);

					DEBUG_ASSERT(node.state == NodeState::Loading, "Node should be in deleting state");

					node.state = NodeState::Loaded;
				}

				// Clear previous commands. If you didn't handle them then too bad
				scale.create_commands.clear();
				scale.load_commands.clear();

				// For each command list that is a child of the world
				spatial_world.loaders_query.iter(worker_entity.world()).each([&](const Loader& spatial_loader, const physics3d::Position& position)
				{
					EASY_BLOCK("SingleLoader");

					const uint32_t scale_step = 1 << scale_index;
					const double scale_node_step = scale_step * spatial_world.world.node_size;

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

						Poly node_poly = it->second;
						Node& node = GetNode(spatial_world.world, node_poly);

						// Touch the node so it stays loaded
						node.last_update_time = world_time.frame_start;

						// Load the node if its not loaded yet
						switch (node.state)
						{
						case NodeState::Unloaded:
							if (scale.load_commands.size() < k_max_frame_load_commands)
							{
								scale.load_commands.push_back(node_poly);
								node.state = NodeState::Loading;
							}
							break;
						}
					});
				});
			}
		});

		// System to initialize spatial nodes that have been added
		world.system<WorldMarker, const sim::GlobalTime>(DEBUG_ONLY("WorldCreateNodes"))
			.multi_threaded()
			.term_at(1).src<sim::GlobalTime>()
			.each([](flecs::entity entity, WorldMarker& spatial_world, const sim::GlobalTime& world_time)
		{
			EASY_BLOCK("WorldCreateNodes");

			DEBUG_ASSERT(spatial_world.world.max_scale > 0, "The spatial world should have at least one scale");

			for (size_t scale_index = 0; scale_index < spatial_world.world.max_scale; scale_index++)
			{
				EASY_BLOCK("ScaleCreateNodes");

				Scale& scale = GetScale(spatial_world.world, scale_index);

				// For each create command
				for (const godot::Vector3i& pos : scale.create_commands)
				{
					// Try and create the node
					auto&& [it, emplaced] = scale.nodes.try_emplace(pos, spatial_world.world.node_type.CreatePoly());

					DEBUG_ASSERT(emplaced, "The node should have been emplaced. We must have had a duplicate command.");

					Poly node_poly = it->second;
					Node& node = GetNode(spatial_world.world, node_poly);

					// Initialize the node
					node.coord = Coord(pos, scale_index);
					node.last_update_time = world_time.frame_start;
					node.state = NodeState::Unloaded;

					InitializeNode(spatial_world.world, node_poly, scale_index);
				}
			}
		});

		// System to delete spatial nodes that have been marked to unload
		world.system<WorldMarker>(DEBUG_ONLY("WorldDestroyNodes"))
			.multi_threaded()
			.each([](flecs::entity entity, WorldMarker& spatial_world)
		{
			EASY_BLOCK("WorldDestroyNodes");

			for (size_t scale_index = 0; scale_index < spatial_world.world.max_scale; scale_index++)
			{
				EASY_BLOCK("ScaleDestroyNodes");

				Scale& scale = GetScale(spatial_world.world, scale_index);

				// For each destroy command of the scale
				for (Poly node_poly : scale.destroy_commands)
				{
					Node& node = GetNode(spatial_world.world, node_poly);

					DEBUG_ASSERT(node.state == NodeState::Deleting, "Node should be in deleting state");

					UninitializeNode(spatial_world.world, node_poly, scale_index);

					spatial_world.world.node_type.DestroyPoly(node_poly);

					scale.nodes.erase(node.coord.pos);
				}
			}
		});

		// System to delete spatial nodes that have been marked to unload
		world.system<ScaleMarker, WorldMarker>(DEBUG_ONLY("EntityUpdateParentNode"))
			.multi_threaded()
			.term_at(1).up(flecs::ChildOf)
			.each([](flecs::entity entity, ScaleMarker& scale_marker, WorldMarker& spatial_world)
		{
			flecs::world stage = entity.world();

			Scale& scale = GetScale(spatial_world.world, scale_marker.scale);
					
			scale_marker.update_parent_node_query.iter(stage).each([&](flecs::entity entity, Entity& spatial_entity, const physics3d::Position& position)
			{
				godot::Vector3i new_position = position.position / (1 >> scale.scale);

				if (spatial_entity.node_pos == new_position)
				{
					return;
				}

				auto it = scale.nodes.find(spatial_entity.node_pos);

				// Remove from entity list of previous node at the previous position
				if (it != scale.nodes.end())
				{
					Poly node_poly = it->second;
					Node& node = GetNode(spatial_world.world, node_poly);

					auto remove_begin = std::remove(node.entities.begin(), node.entities.end(), entity);

					node.entities.erase(remove_begin, node.entities.end());
				}

				// Update position
				spatial_entity.node_pos = new_position;

				// Add to entity list of the node at the position
				it = scale.nodes.find(spatial_entity.node_pos);

				if (it != scale.nodes.end())
				{
					Poly node_poly = it->second;
					Node& node = GetNode(spatial_world.world, node_poly);

					node.entities.push_back(entity);
				}
			});
		});
	}

	void InitializeWorldScales(flecs::entity world_entity, WorldMarker& spatial_world)
	{
		flecs::scoped_world scope = world_entity.scope();

		for (uint8_t scale_index = 0; scale_index < spatial_world.world.max_scale; scale_index++)
		{
			spatial_world.world.scales[scale_index] = spatial_world.world.scale_type.CreatePoly();

			flecs::entity marker_entity = scope.entity();

#if defined(DEBUG_ENABLED)
			marker_entity.set_name(godot::vformat("ScaleMarker%d", scale_index).utf8());
#endif

			marker_entity.set(ScaleMarker{ scale_index });
		}
	}
}