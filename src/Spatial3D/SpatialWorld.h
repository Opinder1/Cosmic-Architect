#pragma once

#include "SpatialPoly.h"

#include "Entity/EntityPoly.h"

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
	constexpr const uint8_t k_node_no_parent = UINT8_MAX;

	enum class NodeState : uint8_t
	{
		Invalid,
		Loading, // Has a load command
		Merging, // Has a merge command
		Loaded, // Fully loaded
		Unmerging, // Has a unmerge command
		Unloading, // Has a unload command
	};

	struct NodeCommand
	{
		NodePtr node;
		uint16_t task_count = 0;
	};

	// A single node in a spatial world. This is meant to be inherited from for custom data
	struct Node : Nocopy, Nomove
	{
		godot::Vector3i position;
		uint8_t scale_index = 0;

		NodeState state = NodeState::Invalid;

		uint8_t parent_index = k_node_no_parent; // The index we are in our parent
		uint8_t children_mask = 0; // Each bit determines a child [0-7]
		uint8_t neighbour_mask = 0; // Each bit determines a neighbour [0-5]

		uint32_t num_observers = 0; // Number of observers looking at me (1 for write, more than 1 means shared read)
		uint32_t network_version = 0; // The version of this node. We use this to check if we should update to a newer version if there is one
		Clock::time_point last_update_time; // Time since a loader last updated our unload timer

		NodePtr parent = nullptr; // Octree parent

		NodePtr children[8] = { nullptr }; // Octree children

		NodePtr neighbours[6] = { nullptr }; // Fast access of neighbours of same scale

		robin_hood::unordered_set<entity::Ref> entities;
	};

	using NodeMap = robin_hood::unordered_map<godot::Vector3i, NodePtr>;

	// A level of detail map for a world. The world will have multiple of these
	struct Scale : Nocopy, Nomove
	{
		WorldPtr world;

		uint8_t index = 0;

		NodeMap nodes;

		robin_hood::unordered_set<entity::Ref> entities;
	};

	struct PartialScale : Nocopy, Nomove
	{
		// Command lists. We use these to limit operations currently in progress
		std::vector<NodeCommand> load_commands;
		std::vector<NodeCommand> merge_commands;
		std::vector<NodeCommand> unmerge_commands;
		std::vector<NodeCommand> unload_commands;
	};

	// A spatial database which has an octree like structure with neighbour pointers and hash maps for each lod. 
	struct World : Nocopy, Nomove
	{
		ScaleType* scale_type = nullptr;
		NodeType* node_type = nullptr;

		uint8_t max_scale = 0;
		uint8_t node_size = 1;

		std::array<ScalePtr, k_max_world_scale> scales;

		robin_hood::unordered_set<entity::Ref> entities;
	};

	struct BoundedWorld : Nocopy, Nomove
	{
		godot::AABB bounds;
	};

	struct PartialWorld : Nocopy, Nomove
	{
		Clock::duration node_keepalive = 10s;

		// Optional entities that act as areas where nodes are loaded around
		robin_hood::unordered_set<entity::Ref> loaders;
	};

	using ScaleCB = cb::Callback<void(ScalePtr)>;
	using NodeCommandCB = cb::Callback<void(NodePtr, uint16_t&)>;

	WorldPtr GetWorld(ScalePtr scale);

	// Get a scale in a world given a position
	ScalePtr GetScale(WorldPtr spatial_world, uint8_t scale_index);

	// Get a node in a world at a position and scale
	NodePtr GetNode(WorldPtr world, godot::Vector3i position, uint8_t scale_index);

	// Create a new spatial world given provided types
	WorldPtr CreateWorld(WorldType& world_type, ScaleType& scale_type, NodeType& node_type, uint8_t max_scale);

	// Destroy a spatial world
	void DestroyWorld(WorldPtr world);

	void AddLoader(WorldPtr world, entity::Ref loader);

	void RemoveLoader(WorldPtr world, entity::Ref loader);

	void AddEntity(WorldPtr world, entity::Ref entity);

	void RemoveEntity(WorldPtr world, entity::Ref entity);

	void WorldForEachScale(WorldPtr world, ScaleCB callback);

	void ScaleDoNodeCommands(ScalePtr scale, NodeState state, NodeCommandCB callback);

	// Execute all node create commands a world has. Thread safe for that world
	void WorldDoNodeLoadCommands(WorldPtr world, Clock::time_point frame_start_time);

	// Execute all node destroy commands a world has. Thread safe for that world
	void WorldDoNodeUnloadCommands(WorldPtr world);

	// Add commands to load all nodes around loaders. Thread safe for that scale
	void ScaleLoadNodesAroundLoaders(ScalePtr scale, Clock::time_point frame_start_time);

	// Add commands to unload nodes that are not near loaders. Thread safe for that scale
	void ScaleUnloadUnutilizedNodes(ScalePtr scale, Clock::time_point frame_start_time);

	// Update the scale entities should be in based on their position. Thread safe for that world
	void WorldUpdateEntityScales(WorldPtr world);

	// Update the node entities should be in based on their position. Thread safe for that scale
	void ScaleUpdateEntityNodes(ScalePtr scale);
}