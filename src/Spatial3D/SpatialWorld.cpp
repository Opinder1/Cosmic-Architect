#include "SpatialComponents.h"
#include "SpatialCoord.h"
#include "SpatialTraverse.h"

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

	const Scale& GetScale(const World& world, uint8_t scale_index)
	{
		DEBUG_ASSERT(scale_index < world.max_scale, "Requested scale out of range");
		return *world.scales[scale_index];
	}

	Scale& GetScale(World& world, uint8_t scale_index)
	{
		DEBUG_ASSERT(scale_index < world.max_scale, "Requested scale out of range");
		return *world.scales[scale_index];
	}

	const Node* GetNode(const World& world, Coord coord)
	{
		DEBUG_ASSERT(coord.scale < k_max_world_scale, "The coordinates scale is out of range");

		const Scale& scale = GetScale(world, coord.scale);

		NodeMap::const_iterator it = scale.nodes.find(coord.pos);

		if (it == scale.nodes.end())
		{
			return nullptr;
		}

		return it->second;
	}

	Node* GetNode(World& world, Coord coord)
	{
		return const_cast<Node*>(GetNode(world, coord));
	}

	void InitializeNode(World& world, Node& node, uint8_t scale_index)
	{
		Scale& scale = GetScale(world, scale_index);

		for (uint8_t neighbour_index = 0; neighbour_index < 6; neighbour_index++)
		{
			godot::Vector3i neighbour_pos = node.coord.pos + node_neighbour_offsets[neighbour_index];

			NodeMap::iterator it = scale.nodes.find(neighbour_pos);

			if (it != scale.nodes.end())
			{
				Node* neighbour_node = it->second;

				node.neighbours[neighbour_index] = neighbour_node;
				node.neighbour_mask |= 1 << neighbour_index;

				neighbour_node->neighbours[5 - neighbour_index] = &node;
				neighbour_node->neighbour_mask |= 1 << (5 - neighbour_index);
			}
		}

		if (scale_index < world.max_scale - 1)
		{
			Scale& parent_scale = GetScale(world, scale_index + 1);

			Coord parent_pos = node.coord.GetParent();

			NodeMap::iterator it = parent_scale.nodes.find(parent_pos.pos);

			if (it != parent_scale.nodes.end())
			{
				Node* parent_node = it->second;

				node.parent = parent_node;
				node.parent_index = GetNodeParentIndex(node.coord.GetParentRelPos());

				DEBUG_ASSERT(node.parent_index < 8, "The parent index is out of range");

				parent_node->children[node.parent_index] = &node;
				parent_node->children_mask |= 1 << node.parent_index;
			}
		}

		if (scale_index > 0)
		{
			Scale& child_scale = GetScale(world, scale_index - 1);

			for (uint8_t child_index = 0; child_index < 8; child_index++)
			{
				Coord child_pos = node.coord.GetBottomLeftChild();
				child_pos.pos += node_child_offsets[child_index];

				NodeMap::iterator it = child_scale.nodes.find(child_pos.pos);

				if (it != child_scale.nodes.end())
				{
					Node* child_node = it->second;

					node.children[child_index] = child_node;
					node.children_mask |= 1 << child_index;

					child_node->parent = &node;
					child_node->parent_index = child_index;
				}
			}
		}
	}

	void UninitializeNode(World& world, Node& node, uint8_t scale_index)
	{
		for (uint8_t neighbour_index = 0; neighbour_index < 6; neighbour_index++)
		{
			if (Node* neighbour_node = node.neighbours[neighbour_index])
			{
				neighbour_node->neighbours[5 - neighbour_index] = nullptr;
				neighbour_node->neighbour_mask &= ~(1 << (5 - neighbour_index));
			}
		}

		if (scale_index < world.max_scale - 1)
		{
			if (Node* parent_node = node.parent)
			{
				parent_node->children[node.parent_index] = nullptr;
				parent_node->children_mask &= ~(1 << node.parent_index);
			}
		}

		if (scale_index > 0)
		{
			for (uint8_t child_index = 0; child_index < 8; child_index++)
			{
				if (Node* child_node = node.children[child_index])
				{
					child_node->parent = nullptr;
					child_node->parent_index = k_node_no_parent;
				}
			}
		}
	}

	void InitializeWorld(World& world)
	{

	}

	void ShutdownWorld(World& world)
	{
		for (size_t scale_index = 0; scale_index < world.max_scale; scale_index++)
		{
			Scale& scale = GetScale(world, scale_index);

			DEBUG_ASSERT(scale.create_commands.empty(), "All commands should have been destroyed before destroying the world");
			DEBUG_ASSERT(scale.load_commands.empty(), "All commands should have been destroyed before destroying the world");
			DEBUG_ASSERT(scale.unload_commands.empty(), "All commands should have been destroyed before destroying the world");
			DEBUG_ASSERT(scale.destroy_commands.empty(), "All commands should have been destroyed before destroying the world");
			DEBUG_ASSERT(scale.nodes.empty(), "All nodes should have been destroyed before destroying the world");
		}
	}

	void WorldSetMaxScale(Types& types, World& world, size_t max_scale)
	{
		if (max_scale > world.max_scale)
		{
			for (uint8_t scale_index = world.max_scale; scale_index < max_scale; scale_index++)
			{
				world.scales[scale_index] = types.scale_type.CreatePoly();
			}
		}
		else
		{
			for (uint8_t scale_index = max_scale; scale_index < world.max_scale; scale_index++)
			{
				Scale& scale = GetScale(world, scale_index);

				// Clear previous commands. If you didn't handle them then too bad
				scale.unload_commands.clear();
				scale.destroy_commands.clear();

				// For each node in the scale
				for (auto&& [pos, node] : scale.nodes)
				{
					switch (node->state)
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

				types.scale_type.DestroyPoly(world.scales[scale_index]);
			}
		}

		world.max_scale = max_scale;

	}

	void WorldCreateNodes(Types& types, World& world, const sim::CFrame& frame)
	{
		DEBUG_ASSERT(world.max_scale > 0, "The spatial world should have at least one scale");

		for (size_t scale_index = 0; scale_index < world.max_scale; scale_index++)
		{
			EASY_BLOCK("ScaleCreateNodes");

			Scale& scale = GetScale(world, scale_index);

			// For each create command
			for (const godot::Vector3i& pos : scale.create_commands)
			{
				// Try and create the node
				auto&& [it, emplaced] = scale.nodes.try_emplace(pos, types.node_type.CreatePoly());

				DEBUG_ASSERT(emplaced, "The node should have been emplaced. We must have had a duplicate command.");

				Node* node = it->second;

				// Initialize the node
				node->coord = Coord(pos, scale_index);
				node->last_update_time = frame.frame_start_time;
				node->state = NodeState::Unloaded;

				InitializeNode(world, *node, scale_index);
			}
		}
	}

	void WorldDestroyNodes(Types& types, World& world)
	{
		for (size_t scale_index = 0; scale_index < world.max_scale; scale_index++)
		{
			EASY_BLOCK("ScaleDestroyNodes");

			Scale& scale = GetScale(world, scale_index);

			// For each destroy command of the scale
			for (Node* node : scale.destroy_commands)
			{
				DEBUG_ASSERT(node->state == NodeState::Deleting, "Node should be in deleting state");

				UninitializeNode(world, *node, scale_index);

				types.node_type.DestroyPoly(node);

				scale.nodes.erase(node->coord.pos);
			}
		}
	}

	void LoaderLoadNodes(Scale& scale, const Loader& loader, const sim::CFrame& frame, double scale_node_step)
	{
		EASY_BLOCK("SingleLoader");

		if (scale.index < loader.min_lod || scale.index > loader.max_lod)
		{
			return;
		}

		// For each node in the sphere of the loader
		ForEachCoordInSphere(loader.position / scale_node_step, loader.dist_per_lod, [&](godot::Vector3i pos)
		{
			NodeMap::iterator it = scale.nodes.find(pos);

			if (it == scale.nodes.end())
			{
				scale.create_commands.push_back(pos); // Create commands should immediately be executed this frame so don't worry about duplicates
				return;
			}

			Node* node = it->second;

			// Touch the node so it stays loaded
			node->last_update_time = frame.frame_start_time;

			// Load the node if its not loaded yet
			switch (node->state)
			{
			case NodeState::Unloaded:
				if (scale.load_commands.size() < k_max_frame_load_commands)
				{
					scale.load_commands.push_back(node);
					node->state = NodeState::Loading;
				}
				break;
			}
		});
	}

	void ScaleLoadNodes(const World& world, Scale& scale, const sim::CFrame& frame)
	{
		// Finish the previous load commands
		for (Node* node : scale.load_commands)
		{
			DEBUG_ASSERT(node->state == NodeState::Loading, "Node should be in deleting state");

			node->state = NodeState::Loaded;
		}

		// Clear previous commands. If you didn't handle them then too bad
		scale.create_commands.clear();
		scale.load_commands.clear();

		const uint32_t scale_step = 1 << scale.index;
		const double scale_node_step = scale_step * world.node_size;

		// For each command list that is a child of the world
		for (const Loader* loader : world.loaders)
		{
			LoaderLoadNodes(scale, *loader, frame, scale_node_step);
		}
	}

	void UnloadNode(Scale& scale, Node& node, const sim::CFrame& frame, Clock::duration node_keepalive)
	{
		// Check if node hasn't been touched in too long
		if (frame.frame_start_time - node.last_update_time < node_keepalive)
		{
			return;
		}

		// Move the entity along to deletion
		switch (node.state)
		{
		case NodeState::Unloaded:
			if (scale.destroy_commands.size() < k_max_frame_destroy_commands)
			{
				scale.destroy_commands.push_back(&node);
				node.state = NodeState::Deleting;
			}
			break;

		case NodeState::Loaded:
			if (scale.unload_commands.size() < k_max_frame_unload_commands)
			{
				scale.unload_commands.push_back(&node);
				node.state = NodeState::Unloading;
			}
			break;
		}
	}

	void ScaleUnloadNodes(const World& world, Scale& scale, const sim::CFrame& frame)
	{
		// Clear previous commands. If you didn't handle them then too bad
		scale.unload_commands.clear();
		scale.destroy_commands.clear();

		// For each node in the scale
		for (auto&& [pos, node] : scale.nodes)
		{
			UnloadNode(scale, *node, frame, world.node_keepalive);
		}
	}

	void WorldUpdateEntityScales(World& world)
	{
		for (Entity* entity : world.entities)
		{
			if (entity->last_scale == entity->scale)
			{
				continue;
			}

			godot::Vector3i required_node_pos = entity->position / (1 >> entity->scale);

			Scale& old_scale = GetScale(world, entity->last_scale);
			Scale& new_scale = GetScale(world, entity->scale);

			auto old_it = old_scale.nodes.find(entity->last_node_pos);
			auto new_it = new_scale.nodes.find(required_node_pos);

			DEBUG_ASSERT(old_it != old_scale.nodes.end(), "The current node should be loaded. If the node was unloaded it should have detached this entity");

			// If the required node is not loaded then wait until it is loaded
			if (new_it != new_scale.nodes.end())
			{
				old_scale.entities.erase(entity);
				new_scale.entities.insert(entity);

				old_it->second->entities.erase(entity);
				new_it->second->entities.insert(entity);

				entity->scale = entity->last_scale;
				entity->last_node_pos = required_node_pos;
			}
		}
	}

	void ScaleUpdateEntityNodes(const World& world, Scale& scale)
	{
		for (Entity* entity : scale.entities)
		{
			godot::Vector3i required_node_pos = entity->position / (1 >> scale.index);

			if (entity->last_node_pos == required_node_pos)
			{
				continue;
			}

			auto old_it = scale.nodes.find(entity->last_node_pos);
			auto new_it = scale.nodes.find(required_node_pos);

			DEBUG_ASSERT(old_it != scale.nodes.end(), "The current node should be loaded. If the node was unloaded it should have detached this entity");

			// If the required node is not loaded then wait until it is loaded
			if (new_it != scale.nodes.end())
			{
				old_it->second->entities.erase(entity);
				new_it->second->entities.insert(entity);

				entity->last_node_pos = required_node_pos;
			}
		}
	}
}