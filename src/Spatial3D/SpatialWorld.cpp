#include "SpatialComponents.h"
#include "SpatialTraverse.h"

#include "Util/GodotOperators.h"

#include <easy/profiler.h>

namespace voxel_game::spatial3d
{
	const godot::Vector3i node_neighbour_offsets[6] =
	{
		{-1, 0, 0},
		{0, -1, 0},
		{0, 0, -1},
		{0, 0, 1},
		{0, 1, 0},
		{1, 0, 0},
	};

	// Convert a nodes index in the parent into a position in the parent. Does the opposite of GetNodeParentIndex().
	const godot::Vector3i node_child_offsets[8] =
	{
		{0, 0, 0},
		{1, 0, 0},
		{0, 1, 0},
		{1, 1, 0},
		{0, 0, 1},
		{1, 0, 1},
		{0, 1, 1},
		{1, 1, 1},
	};

	// Convert a node child pos to an index in the parent. Does the opposite of node_child_offsets[].
	uint8_t GetNodeParentIndex(godot::Vector3i pos)
	{
		return (pos.x & 0x1) + ((pos.y & 0x1) * 2) + ((pos.z & 0x1) * 4);
	}

	WorldPtr GetWorld(ScalePtr scale)
	{
		return scale->*&Scale::world;
	}

	ScalePtr GetScale(WorldPtr world, uint8_t scale_index)
	{
		DEBUG_ASSERT(scale_index < world->*&World::max_scale, "Requested scale out of range");

		return (world->*&World::scales)[scale_index];
	}

	NodePtr GetNode(WorldPtr world, godot::Vector3i position, uint8_t scale_index)
	{
		DEBUG_ASSERT(scale_index < k_max_world_scale, "The coordinates scale is out of range");

		ScalePtr scale = GetScale(world, scale_index);

		NodeMap::const_iterator it = (scale->*&Scale::nodes).find(position);

		if (it == (scale->*&Scale::nodes).end())
		{
			return nullptr;
		}

		return it->second;
	}

	std::vector<NodeCommand> spatial3d::PartialScale::* GetStateCommands(NodeState state)
	{
		switch (state)
		{
		case NodeState::Loading:
			return &spatial3d::PartialScale::load_commands;

		case NodeState::Unloading:
			return &spatial3d::PartialScale::unload_commands;

		default:
			return nullptr;
		}
	}

	void WorldForEachScale(WorldPtr world, ScaleCB callback)
	{
		DEBUG_THREAD_CHECK_READ(world.Data());

		DEBUG_ASSERT(world->*&spatial3d::World::max_scale > 0, "The spatial world should have at least one scale");

		for (size_t scale_index = 0; scale_index < world->*&spatial3d::World::max_scale; scale_index++)
		{
			spatial3d::ScalePtr scale = spatial3d::GetScale(world, scale_index);

			callback(scale);
		}
	}

	void ScaleDoNodeCommands(ScalePtr scale, NodeState state, NodeCommandCB callback)
	{
		DEBUG_THREAD_CHECK_WRITE(scale.Data());

		// For each create command
		for (spatial3d::NodeCommand& command : scale->*GetStateCommands(state))
		{
			callback(command.node, command.task_count);
		}
	}

	void LinkNode(WorldPtr world, NodePtr node, uint8_t scale_index)
	{
		DEBUG_THREAD_CHECK_WRITE(world.Data());

		ScalePtr scale = GetScale(world, scale_index);

		for (uint8_t neighbour_index = 0; neighbour_index < 6; neighbour_index++)
		{
			godot::Vector3i neighbour_pos = node->*&Node::position + node_neighbour_offsets[neighbour_index];

			NodeMap::iterator it = (scale->*&Scale::nodes).find(neighbour_pos);

			if (it != (scale->*&Scale::nodes).end())
			{
				NodePtr neighbour_node = it->second;

				(node->*&Node::neighbours)[neighbour_index] = neighbour_node;
				node->*&Node::neighbour_mask |= 1 << neighbour_index;

				(neighbour_node->*&Node::neighbours)[5 - neighbour_index] = node;
				neighbour_node->*&Node::neighbour_mask |= 1 << (5 - neighbour_index);
			}
		}

		if (scale_index < world->*&World::max_scale - 1)
		{
			ScalePtr parent_scale = GetScale(world, scale_index + 1);

			godot::Vector3i parent_pos = node->*&Node::position >> 1;

			NodeMap::iterator it = (parent_scale->*&Scale::nodes).find(parent_pos);

			if (it != (parent_scale->*&Scale::nodes).end())
			{
				NodePtr parent_node = it->second;

				node->*&Node::parent = parent_node;
				node->*&Node::parent_index = GetNodeParentIndex(node->*&Node::position);

				DEBUG_ASSERT(node->*&Node::parent_index < 8, "The parent index is out of range");

				(parent_node->*&Node::children)[node->*&Node::parent_index] = node;
				parent_node->*&Node::children_mask |= 1 << node->*&Node::parent_index;
			}
		}

		if (scale_index > 0)
		{
			ScalePtr child_scale = GetScale(world, scale_index - 1);

			for (uint8_t child_index = 0; child_index < 8; child_index++)
			{
				godot::Vector3i child_pos = node->*&Node::position << 1;
				child_pos += node_child_offsets[child_index];

				NodeMap::iterator it = (child_scale->*&Scale::nodes).find(child_pos);

				if (it != (child_scale->*&Scale::nodes).end())
				{
					NodePtr child_node = it->second;

					(node->*&Node::children)[child_index] = child_node;
					node->*&Node::children_mask |= 1 << child_index;

					child_node->*&Node::parent = node;
					child_node->*&Node::parent_index = child_index;
				}
			}
		}
	}

	void UnlinkNode(WorldPtr world, NodePtr node, uint8_t scale_index)
	{
		DEBUG_THREAD_CHECK_WRITE(world.Data());

		for (uint8_t neighbour_index = 0; neighbour_index < 6; neighbour_index++)
		{
			if (NodePtr neighbour_node = (node->*&Node::neighbours)[neighbour_index])
			{
				(neighbour_node->*&Node::neighbours)[5 - neighbour_index] = nullptr;
				neighbour_node->*&Node::neighbour_mask &= ~(1 << (5 - neighbour_index));
			}
		}

		if (scale_index < world->*&World::max_scale - 1)
		{
			if (NodePtr parent_node = node->*&Node::parent)
			{
				(parent_node->*&Node::children)[node->*&Node::parent_index] = nullptr;
				parent_node->*&Node::children_mask &= ~(1 << node->*&Node::parent_index);
			}
		}

		if (scale_index > 0)
		{
			for (uint8_t child_index = 0; child_index < 8; child_index++)
			{
				if (NodePtr child_node = (node->*&Node::children)[child_index])
				{
					child_node->*&Node::parent = nullptr;
					child_node->*&Node::parent_index = k_node_no_parent;
				}
			}
		}
	}

	WorldPtr CreateWorld(TypeData& type)
	{
		DEBUG_THREAD_CHECK_WRITE(&type);

		WorldPtr world = type.world_type.CreatePoly();

		world->*&World::type = &type;

		type.worlds.push_back(world);

		for (uint8_t scale_index = world->*&World::max_scale; scale_index < type.max_scale; scale_index++)
		{
			ScalePtr scale = (world->*&World::type)->scale_type.CreatePoly();

			scale->*&Scale::world = world;

			scale->*&Scale::index = scale_index;

			(world->*&World::scales)[scale_index] = scale;

			type.scales.push_back(scale);
		}

		world->*&World::max_scale = type.max_scale;

		return world;
	}

	void UnloadWorld(WorldPtr world)
	{
		DEBUG_THREAD_CHECK_WRITE(world.Data());

		world->*&World::unloading = true;
	}

	void DestroyWorld(WorldPtr world)
	{
		DEBUG_THREAD_CHECK_WRITE(world.Data());

		TypeData& type = *(world->*&World::type);

		DEBUG_THREAD_CHECK_WRITE(&type);

		WorldForEachScale(world, [&](ScalePtr scale)
		{
			DEBUG_ASSERT((scale->*&Scale::nodes).empty(), "All nodes should have been destroyed before destroying the world");
			if (scale.Has<PartialScale>())
			{
				DEBUG_ASSERT((scale->*&PartialScale::load_commands).empty(), "All commands should have been destroyed before destroying the world");
				DEBUG_ASSERT((scale->*&PartialScale::unload_commands).empty(), "All commands should have been destroyed before destroying the world");
			}

			unordered_erase(type.scales, scale);

			type.scale_type.DestroyPoly(scale);
		});

		unordered_erase(type.worlds, world);

		type.world_type.DestroyPoly(world);
	}

	bool IsWorldUnloading(WorldPtr world)
	{
		DEBUG_THREAD_CHECK_READ(world.Data());

		return world->*&World::unloading;
	}

	size_t GetNodeCount(WorldPtr world)
	{
		DEBUG_THREAD_CHECK_READ(world.Data());

		size_t nodes = 0;
		WorldForEachScale(world, [&nodes](ScalePtr scale)
		{
			nodes += (scale->*&Scale::nodes).size();
		});
		return nodes;
	}

	void AddEntity(WorldPtr world, entity::WRef entity)
	{
		(world->*&World::entities).emplace(entity);

		if (entity.Has<CLoader>())
		{
			(world->*&PartialWorld::loaders).emplace(entity);
		}
	}

	void RemoveEntity(WorldPtr world, entity::WRef entity)
	{
		(world->*&World::entities).erase(entity::Ref(entity));

		if (entity.Has<CLoader>())
		{
			(world->*&PartialWorld::loaders).erase(entity::Ref(entity));
		}
	}

	void WorldDoNodeLoadCommands(WorldPtr world, Clock::time_point frame_start_time)
	{
		DEBUG_THREAD_CHECK_WRITE(world.Data());
		EASY_BLOCK("WorldDoNodeLoadCommands");

		DEBUG_ASSERT(world->*&World::max_scale > 0, "The spatial world should have at least one scale");

		WorldForEachScale(world, [&](ScalePtr scale)
		{
			// For each create command
			ScaleDoNodeCommands(scale, NodeState::Loading, [&](NodePtr node, uint16_t& task_count)
			{
				DEBUG_ASSERT(node->*&PartialNode::state == NodeState::Loading, "Node should be in loading state");

				if (task_count > 0)
				{
					return;
				}

				// Initialize the node
				node->*&PartialNode::state = NodeState::Loaded;
				node->*&PartialNode::last_update_time = frame_start_time;

				NodeMap::iterator it = (scale->*&Scale::nodes).find(node->*&Node::position);
				DEBUG_ASSERT(it != (scale->*&Scale::nodes).end(), "The node doesn't exist");

				it->second = node;

				LinkNode(world, node, scale->*&Scale::index);
			});

			(scale->*&PartialScale::load_commands).clear();
		});
	}

	void WorldDoNodeUnloadCommands(WorldPtr world)
	{
		DEBUG_THREAD_CHECK_WRITE(world.Data());
		EASY_BLOCK("WorldDoNodeUnloadCommands");

		TypeData& type = *(world->*&World::type);

		WorldForEachScale(world, [&](ScalePtr scale)
		{
			// For each destroy command of the scale
			ScaleDoNodeCommands(scale, NodeState::Unloading, [&](NodePtr node, uint16_t& task_count)
			{
				DEBUG_ASSERT(node->*&PartialNode::state == NodeState::Unloading, "Node should be in unloading state");

				if (task_count > 0)
				{
					return;
				}

				UnlinkNode(world, node, scale->*&Scale::index);

				(scale->*&Scale::nodes).erase(node->*&Node::position);

				type.node_type.DestroyPoly(node);
			});

			(scale->*&PartialScale::unload_commands).clear();
		});
	}

	void LoaderLoadNodes(ScalePtr scale, entity::WRef loader, Clock::time_point frame_start_time, double scale_node_step)
	{
		DEBUG_THREAD_CHECK_WRITE(scale.Data());
		EASY_BLOCK("SingleLoader");

		if (scale->*&Scale::index < loader->*&CLoader::min_lod || scale->*&Scale::index > loader->*&CLoader::max_lod)
		{
			return;
		}

		WorldPtr world = scale->*&Scale::world;

		TypeData& type = *(world->*&World::type);

		// For each node in the sphere of the loader
		ForEachCoordInSphere(loader->*&physics3d::CPosition::position / scale_node_step, loader->*&CLoader::dist_per_lod, [&](godot::Vector3i pos)
		{
			// Try and create the node
			auto&& [it, emplaced] = (scale->*&Scale::nodes).try_emplace(pos, nullptr);

			if (emplaced) // Node didn't already exist
			{
				NodePtr node = type.node_type.CreatePoly();

				node->*&Node::position = pos;
				node->*&Node::scale_index = scale->*&Scale::index;
				node->*&PartialNode::state = NodeState::Loading;
				node->*&PartialNode::last_update_time = frame_start_time;

				(scale->*&PartialScale::load_commands).push_back(NodeCommand{ node });
				return;
			}

			NodePtr node = it->second;

			// Stop if we are still creating this node
			if (!node)
			{
				return;
			}

			// Touch the node so it stays loaded
			node->*&PartialNode::last_update_time = frame_start_time;
		});
	}

	void ScaleLoadNodesAroundLoaders(ScalePtr scale, Clock::time_point frame_start_time)
	{
		DEBUG_THREAD_CHECK_WRITE(scale.Data());

		WorldPtr world = scale->*&Scale::world;

		const uint32_t scale_step = 1 << scale->*&Scale::index;
		const double scale_node_step = scale_step * world->*&World::node_size;

		// For each command list that is a child of the world
		for (entity::WRef loader : world->*&PartialWorld::loaders)
		{
			LoaderLoadNodes(scale, loader, frame_start_time, scale_node_step);
		}
	}

	void ScaleUnloadUnutilizedNodes(ScalePtr scale, Clock::time_point frame_start_time)
	{
		DEBUG_THREAD_CHECK_WRITE(scale.Data());

		WorldPtr world = scale->*&Scale::world;

		// For each node in the scale
		for (auto&& [pos, node] : scale->*&Scale::nodes)
		{
			if (!node)
			{
				continue;
			}

			// Check if node hasn't been touched in too long
			bool node_untouched = frame_start_time - node->*&PartialNode::last_update_time > world->*&PartialWorld::node_keepalive;

			if (world->*&World::unloading || node_untouched)
			{
				// Move the entity along to deletion
				switch (node->*&PartialNode::state)
				{
				case NodeState::Loaded:
					node->*&PartialNode::state = NodeState::Unloading;

					node = nullptr;

					(scale->*&PartialScale::unload_commands).push_back(NodeCommand{ node });
					break;
				}
			}
		}
	}

	void WorldUpdateEntityScales(WorldPtr world)
	{
		DEBUG_THREAD_CHECK_WRITE(world.Data());

		for (entity::WRef entity : world->*&World::entities)
		{
			if (entity->*&CEntity::last_scale == entity->*&CEntity::scale)
			{
				continue;
			}

			godot::Vector3i required_node_pos = entity->*&CEntity::position / (1 >> entity->*&CEntity::scale);

			ScalePtr old_scale = GetScale(world, entity->*&CEntity::last_scale);
			ScalePtr new_scale = GetScale(world, entity->*&CEntity::scale);

			auto old_it = (old_scale->*&Scale::nodes).find(entity->*&CEntity::last_node_pos);
			auto new_it = (new_scale->*&Scale::nodes).find(required_node_pos);

			DEBUG_ASSERT(old_it != (old_scale->*&Scale::nodes).end(), "The current node should be loaded. If the node was unloaded it should have detached this entity");

			// If the required node is not loaded then wait until it is loaded
			if (new_it != (new_scale->*&Scale::nodes).end())
			{
				(old_scale->*&Scale::entities).erase(entity::Ref(entity));
				(new_scale->*&Scale::entities).insert(entity::Ref(entity));

				(old_it->second->*&Node::entities).erase(entity::Ref(entity));
				(new_it->second->*&Node::entities).insert(entity::Ref(entity));

				entity->*&CEntity::scale = entity->*&CEntity::last_scale;
				entity->*&CEntity::last_node_pos = required_node_pos;
			}
		}
	}

	void ScaleUpdateEntityNodes(ScalePtr scale)
	{
		DEBUG_THREAD_CHECK_WRITE(scale.Data());

		for (entity::WRef entity : scale->*&Scale::entities)
		{
			godot::Vector3i required_node_pos = entity->*&CEntity::position / (1 >> scale->*&Scale::index);

			if (entity->*&CEntity::last_node_pos == required_node_pos)
			{
				continue;
			}

			auto old_it = (scale->*&Scale::nodes).find(entity->*&CEntity::last_node_pos);
			auto new_it = (scale->*&Scale::nodes).find(required_node_pos);

			DEBUG_ASSERT(old_it != (scale->*&Scale::nodes).end(), "The current node should be loaded. If the node was unloaded it should have detached this entity");

			// If the required node is not loaded then wait until it is loaded
			if (new_it != (scale->*&Scale::nodes).end())
			{
				(old_it->second->*&Node::entities).erase(entity::Ref(entity));
				(new_it->second->*&Node::entities).insert(entity::Ref(entity));

				entity->*&CEntity::last_node_pos = required_node_pos;
			}
		}
	}
}