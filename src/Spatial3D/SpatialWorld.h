#pragma once

#include "SpatialPoly.h"

#include "Entity/EntityPoly.h"

#include "Simulation/SimulationComponents.h"

#include "Util/Nocopy.h"
#include "Util/Util.h"
#include "Util/GodotHash.h"

#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/vector3i.hpp>

#include <robin_hood/robin_hood.h>

#include <array>
#include <vector>

namespace voxel_game::spatial3d
{
	// The max scale that a world can have
	constexpr const uint8_t k_max_world_scale = 16;
	constexpr const size_t k_max_frame_load_commands = 4;
	constexpr const size_t k_max_frame_unload_commands = 4;
	constexpr const size_t k_max_frame_destroy_commands = 4;
	constexpr const uint8_t k_node_no_parent = UINT8_MAX;

	enum class NodeState : uint8_t
	{
		Unloaded, // Created but not loaded
		Loaded, // Fully loaded

		Loading, // Has a load command
		Unloading, // Has a unload command
		Deleting, // Has a delete command
	};

	// A single node in a spatial world. This is meant to be inherited from for custom data
	struct Node : Nocopy, Nomove
	{
		godot::Vector3i position;
		uint8_t scale_index = 0;

		NodeState state = NodeState::Unloaded;

		uint8_t parent_index = k_node_no_parent; // The index we are in our parent
		uint8_t children_mask = 0; // Each bit determines a child [0-7]
		uint8_t neighbour_mask = 0; // Each bit determines a neighbour [0-5]

		uint32_t num_observers = 0; // Number of observers looking at me (1 for write, more than 1 means shared read)
		uint32_t network_version = 0; // The version of this node. We use this to check if we should update to a newer version if there is one
		Clock::time_point last_update_time; // Time since a loader last updated our unload timer

		NodeRef parent = nullptr; // Octree parent

		NodeRef children[8] = { nullptr }; // Octree children

		NodeRef neighbours[6] = { nullptr }; // Fast access of neighbours of same scale

		robin_hood::unordered_set<entity::Ref> entities;
	};

	using NodeMap = robin_hood::unordered_map<godot::Vector3i, NodeRef>;

	// A level of detail map for a world. The world will have multiple of these
	struct Scale : Nocopy, Nomove
	{
		WorldRef world;

		uint8_t index = 0;

		NodeMap nodes;

		robin_hood::unordered_set<entity::Ref> entities;

		// Commands
		std::vector<godot::Vector3i> create_commands;
		std::vector<NodeRef> load_commands;
		std::vector<NodeRef> unload_commands;
		std::vector<NodeRef> destroy_commands;
	};

	// A spatial database which has an octree like structure with neighbour pointers and hash maps for each lod. 
	struct World : Nocopy, Nomove
	{
		ScaleType* scale_type = nullptr;
		NodeType* node_type = nullptr;

		godot::AABB bounds;
		uint8_t max_scale = 0;
		uint8_t node_size = 1;

		Clock::duration node_keepalive = 10s;

		robin_hood::unordered_set<entity::Ref> loaders;

		robin_hood::unordered_set<entity::Ref> entities;

		std::array<ScaleRef, k_max_world_scale> scales;
	};

	ScaleRef GetScale(WorldRef spatial_world, uint8_t scale_index);

	NodeRef GetNode(WorldRef world, godot::Vector3i position, uint8_t scale_index);

	WorldRef CreateWorld(Types& types, uint8_t max_scale);

	void DestroyWorld(WorldRef world);

	void WorldCreateNodes(WorldRef world, Clock::time_point frame_start_time);

	void WorldDestroyNodes(WorldRef world);

	void ScaleLoadNodes(WorldRef world, ScaleRef scale, Clock::time_point frame_start_time);

	void ScaleUnloadNodes(WorldRef world, ScaleRef scale, Clock::time_point frame_start_time);

	void WorldUpdateEntityScales(WorldRef world);

	void ScaleUpdateEntityNodes(WorldRef world, ScaleRef scale);
}