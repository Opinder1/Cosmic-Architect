#include "SpatialComponents.h"
#include "SpatialTraverse.h"

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
		return (pos.x & 0x1) + ((pos.y & 0x1) * 2) + ((pos.z & 0x1) * 4);
	}

	ScaleRef GetScale(WorldRef world, uint8_t scale_index)
	{
		DEBUG_ASSERT(scale_index < world->*&World::max_scale, "Requested scale out of range");
		return (world->*&World::scales)[scale_index];
	}

	NodeRef GetNode(WorldRef world, godot::Vector3i position, uint8_t scale_index)
	{
		DEBUG_ASSERT(scale_index < k_max_world_scale, "The coordinates scale is out of range");

		ScaleRef scale = GetScale(world, scale_index);

		NodeMap::const_iterator it = (scale->*&Scale::nodes).find(position);

		if (it == (scale->*&Scale::nodes).end())
		{
			return nullptr;
		}

		return it->second;
	}

	void InitializeNode(WorldRef world, NodeRef node, uint8_t scale_index)
	{
		ScaleRef scale = GetScale(world, scale_index);

		for (uint8_t neighbour_index = 0; neighbour_index < 6; neighbour_index++)
		{
			godot::Vector3i neighbour_pos = node->*&Node::position + node_neighbour_offsets[neighbour_index];

			NodeMap::iterator it = (scale->*&Scale::nodes).find(neighbour_pos);

			if (it != (scale->*&Scale::nodes).end())
			{
				NodeRef neighbour_node = it->second;

				(node->*&Node::neighbours)[neighbour_index] = neighbour_node;
				node->*&Node::neighbour_mask |= 1 << neighbour_index;

				(neighbour_node->*&Node::neighbours)[5 - neighbour_index] = node;
				neighbour_node->*&Node::neighbour_mask |= 1 << (5 - neighbour_index);
			}
		}

		if (scale_index < world->*&World::max_scale - 1)
		{
			ScaleRef parent_scale = GetScale(world, scale_index + 1);

			godot::Vector3i parent_pos = node->*&Node::position / 2;

			NodeMap::iterator it = (parent_scale->*&Scale::nodes).find(parent_pos);

			if (it != (parent_scale->*&Scale::nodes).end())
			{
				NodeRef parent_node = it->second;

				node->*&Node::parent = parent_node;
				node->*&Node::parent_index = GetNodeParentIndex(node->*&Node::position);

				DEBUG_ASSERT(node->*&Node::parent_index < 8, "The parent index is out of range");

				(parent_node->*&Node::children)[node->*&Node::parent_index] = node;
				parent_node->*&Node::children_mask |= 1 << node->*&Node::parent_index;
			}
		}

		if (scale_index > 0)
		{
			ScaleRef child_scale = GetScale(world, scale_index - 1);

			for (uint8_t child_index = 0; child_index < 8; child_index++)
			{
				godot::Vector3i child_pos = node->*&Node::position * 2;
				child_pos += node_child_offsets[child_index];

				NodeMap::iterator it = (child_scale->*&Scale::nodes).find(child_pos);

				if (it != (child_scale->*&Scale::nodes).end())
				{
					NodeRef child_node = it->second;

					(node->*&Node::children)[child_index] = child_node;
					node->*&Node::children_mask |= 1 << child_index;

					child_node->*&Node::parent = node;
					child_node->*&Node::parent_index = child_index;
				}
			}
		}
	}

	void UninitializeNode(WorldRef world, NodeRef node, uint8_t scale_index)
	{
		for (uint8_t neighbour_index = 0; neighbour_index < 6; neighbour_index++)
		{
			if (NodeRef neighbour_node = (node->*&Node::neighbours)[neighbour_index])
			{
				(neighbour_node->*&Node::neighbours)[5 - neighbour_index] = nullptr;
				neighbour_node->*&Node::neighbour_mask &= ~(1 << (5 - neighbour_index));
			}
		}

		if (scale_index < world->*&World::max_scale - 1)
		{
			if (NodeRef parent_node = node->*&Node::parent)
			{
				(parent_node->*&Node::children)[node->*&Node::parent_index] = nullptr;
				parent_node->*&Node::children_mask &= ~(1 << node->*&Node::parent_index);
			}
		}

		if (scale_index > 0)
		{
			for (uint8_t child_index = 0; child_index < 8; child_index++)
			{
				if (NodeRef child_node = (node->*&Node::children)[child_index])
				{
					child_node->*&Node::parent = nullptr;
					child_node->*&Node::parent_index = k_node_no_parent;
				}
			}
		}
	}

	WorldRef CreateWorld(Types& types, uint8_t max_scale)
	{
		WorldRef world = types.world_type.CreatePoly();

		world->*&World::scale_type = &types.scale_type;
		world->*&World::node_type = &types.node_type;

		WorldSetMaxScale(world, max_scale);

		return world;
	}

	void DestroyWorld(WorldRef world)
	{
		for (size_t scale_index = 0; scale_index < world->*&World::max_scale; scale_index++)
		{
			ScaleRef scale = GetScale(world, scale_index);

			DEBUG_ASSERT((scale->*&Scale::create_commands).empty(), "All commands should have been destroyed before destroying the world");
			DEBUG_ASSERT((scale->*&Scale::load_commands).empty(), "All commands should have been destroyed before destroying the world");
			DEBUG_ASSERT((scale->*&Scale::unload_commands).empty(), "All commands should have been destroyed before destroying the world");
			DEBUG_ASSERT((scale->*&Scale::destroy_commands).empty(), "All commands should have been destroyed before destroying the world");
			DEBUG_ASSERT((scale->*&Scale::nodes).empty(), "All nodes should have been destroyed before destroying the world");
		}

		world.Destroy();
	}

	void WorldSetMaxScale(WorldRef world, size_t max_scale)
	{
		if (max_scale > world->*&World::max_scale)
		{
			for (uint8_t scale_index = world->*&World::max_scale; scale_index < max_scale; scale_index++)
			{
				ScaleRef scale = (world->*&World::scale_type)->CreatePoly();

				scale->*&Scale::index = scale_index;

				(world->*&World::scales)[scale_index] = scale;
			}
		}
		else
		{
			for (uint8_t scale_index = max_scale; scale_index < world->*&World::max_scale; scale_index++)
			{
				ScaleRef scale = GetScale(world, scale_index);

				// Clear previous commands. If you didn't handle them then too bad
				(scale->*&Scale::unload_commands).clear();
				(scale->*&Scale::destroy_commands).clear();

				// For each node in the scale
				for (auto&& [pos, node] : scale->*&Scale::nodes)
				{
					switch (node->*&Node::state)
					{
					case NodeState::Loading:
					case NodeState::Loaded:
					case NodeState::Unloading:
						// Unload node

						// Fallthrough

					case NodeState::Unloaded:
					case NodeState::Deleting:
						// Destroy node
						break;
					}
				}

				scale.Destroy();
			}
		}

		world->*&World::max_scale = max_scale;
	}

	void WorldCreateNodes(WorldRef world, const sim::CFrame& frame)
	{
		DEBUG_ASSERT(world->*&World::max_scale > 0, "The spatial world should have at least one scale");

		for (size_t scale_index = 0; scale_index < world->*&World::max_scale; scale_index++)
		{
			EASY_BLOCK("ScaleCreateNodes");

			ScaleRef scale = GetScale(world, scale_index);

			// For each create command
			for (const godot::Vector3i& pos : scale->*&Scale::create_commands)
			{
				// Try and create the node
				auto&& [it, emplaced] = (scale->*&Scale::nodes).try_emplace(pos, nullptr);

				if (!emplaced) // Node already exists
				{
					continue;
				}

				it->second = (world->*&World::node_type)->CreatePoly();

				NodeRef node = it->second;

				// Initialize the node
				node->*&Node::position = pos;
				node->*&Node::scale_index = scale_index;
				node->*&Node::last_update_time = frame.frame_start_time;
				node->*&Node::state = NodeState::Unloaded;

				InitializeNode(world, node, scale_index);
			}

			(scale->*&Scale::create_commands).clear();
		}
	}

	void WorldDestroyNodes(WorldRef world)
	{
		for (size_t scale_index = 0; scale_index < world->*&World::max_scale; scale_index++)
		{
			EASY_BLOCK("ScaleDestroyNodes");

			ScaleRef scale = GetScale(world, scale_index);

			// For each destroy command of the scale
			for (NodeRef node : scale->*&Scale::destroy_commands)
			{
				DEBUG_ASSERT(node->*&Node::state == NodeState::Deleting, "Node should be in deleting state");

				UninitializeNode(world, node, scale_index);

				(scale->*&Scale::nodes).erase(node->*&Node::position);

				node.Destroy();
			}

			(scale->*&Scale::destroy_commands).clear();
		}
	}

	void LoaderLoadNodes(ScaleRef scale, const Loader& loader, const sim::CFrame& frame, double scale_node_step)
	{
		EASY_BLOCK("SingleLoader");

		if (scale->*&Scale::index < loader.min_lod || scale->*&Scale::index > loader.max_lod)
		{
			return;
		}

		// For each node in the sphere of the loader
		ForEachCoordInSphere(loader.position / scale_node_step, loader.dist_per_lod, [&](godot::Vector3i pos)
		{
			NodeMap::iterator it = (scale->*&Scale::nodes).find(pos);

			if (it == (scale->*&Scale::nodes).end())
			{
				(scale->*&Scale::create_commands).push_back(pos); // Create commands should immediately be executed this frame so don't worry about duplicates
				return;
			}

			NodeRef node = it->second;

			// Touch the node so it stays loaded
			node->*&Node::last_update_time = frame.frame_start_time;

			// Load the node if its not loaded yet
			switch (node->*&Node::state)
			{
			case NodeState::Unloaded:
				if ((scale->*&Scale::load_commands).size() < k_max_frame_load_commands)
				{
					(scale->*&Scale::load_commands).push_back(node);
					node->*&Node::state = NodeState::Loading;
				}
				break;
			}
		});
	}

	void ScaleLoadNodes(WorldRef world, ScaleRef scale, const sim::CFrame& frame)
	{
		// Finish the previous load commands
		for (NodeRef node : scale->*&Scale::load_commands)
		{
			DEBUG_ASSERT(node->*&Node::state == NodeState::Loading, "Node should be in loading state");

			node->*&Node::state = NodeState::Loaded;
		}

		// Clear previous commands. If you didn't handle them then too bad
		(scale->*&Scale::load_commands).clear();

		const uint32_t scale_step = 1 << scale->*&Scale::index;
		const double scale_node_step = scale_step * world->*&World::node_size;

		// For each command list that is a child of the world
		for (const Loader* loader : world->*&World::loaders)
		{
			LoaderLoadNodes(scale, *loader, frame, scale_node_step);
		}
	}

	void UnloadNode(ScaleRef scale, NodeRef node, const sim::CFrame& frame)
	{
		// Move the entity along to deletion
		switch (node->*&Node::state)
		{
		case NodeState::Unloaded:
			if ((scale->*&Scale::destroy_commands).size() < k_max_frame_destroy_commands)
			{
				(scale->*&Scale::destroy_commands).push_back(node);
				node->*&Node::state = NodeState::Deleting;
			}
			break;

		case NodeState::Loaded:
			if ((scale->*&Scale::unload_commands).size() < k_max_frame_unload_commands)
			{
				(scale->*&Scale::unload_commands).push_back(node);
				node->*&Node::state = NodeState::Unloading;
			}
			break;
		}
	}

	void ScaleUnloadNodes(WorldRef world, ScaleRef scale, const sim::CFrame& frame)
	{
		// Finish the previous load commands
		for (NodeRef node : scale->*&Scale::unload_commands)
		{
			DEBUG_ASSERT(node->*&Node::state == NodeState::Unloading, "Node should be in unloading state");

			node->*&Node::state = NodeState::Unloaded;
		}

		// Clear previous commands. If you didn't handle them then too bad
		(scale->*&Scale::unload_commands).clear();

		// For each node in the scale
		for (auto&& [pos, node] : scale->*&Scale::nodes)
		{
			// Check if node hasn't been touched in too long
			if (frame.frame_start_time - node->*&Node::last_update_time > world->*&World::node_keepalive)
			{
				UnloadNode(scale, node, frame);
			}
		}
	}

	void WorldUpdateEntityScales(WorldRef world)
	{
		for (Entity* entity : world->*&World::entities)
		{
			if (entity->last_scale == entity->scale)
			{
				continue;
			}

			godot::Vector3i required_node_pos = entity->position / (1 >> entity->scale);

			ScaleRef old_scale = GetScale(world, entity->last_scale);
			ScaleRef new_scale = GetScale(world, entity->scale);

			auto old_it = (old_scale->*&Scale::nodes).find(entity->last_node_pos);
			auto new_it = (new_scale->*&Scale::nodes).find(required_node_pos);

			DEBUG_ASSERT(old_it != (old_scale->*&Scale::nodes).end(), "The current node should be loaded. If the node was unloaded it should have detached this entity");

			// If the required node is not loaded then wait until it is loaded
			if (new_it != (new_scale->*&Scale::nodes).end())
			{
				(old_scale->*&Scale::entities).erase(entity);
				(new_scale->*&Scale::entities).insert(entity);

				(old_it->second->*&Node::entities).erase(entity);
				(new_it->second->*&Node::entities).insert(entity);

				entity->scale = entity->last_scale;
				entity->last_node_pos = required_node_pos;
			}
		}
	}

	void ScaleUpdateEntityNodes(WorldRef world, ScaleRef scale)
	{
		for (Entity* entity : scale->*&Scale::entities)
		{
			godot::Vector3i required_node_pos = entity->position / (1 >> scale->*&Scale::index);

			if (entity->last_node_pos == required_node_pos)
			{
				continue;
			}

			auto old_it = (scale->*&Scale::nodes).find(entity->last_node_pos);
			auto new_it = (scale->*&Scale::nodes).find(required_node_pos);

			DEBUG_ASSERT(old_it != (scale->*&Scale::nodes).end(), "The current node should be loaded. If the node was unloaded it should have detached this entity");

			// If the required node is not loaded then wait until it is loaded
			if (new_it != (scale->*&Scale::nodes).end())
			{
				(old_it->second->*&Node::entities).erase(entity);
				(new_it->second->*&Node::entities).insert(entity);

				entity->last_node_pos = required_node_pos;
			}
		}
	}
}