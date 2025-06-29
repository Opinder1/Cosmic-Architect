#pragma once

#include "SpatialPoly.h"

#include "Entity/EntityPoly.h"

#include "Util/Nocopy.h"
#include "Util/Util.h"
#include "Util/GodotHash.h"
#include "Util/Callback.h"

#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/vector3i.hpp>

#include <robin_hood/robin_hood.h>

#include <array>
#include <vector>

namespace voxel_game::spatial3d
{
	struct TypeData;

	// The max scale that a world can have
	constexpr const uint8_t k_max_world_scale = 16;
	constexpr const uint8_t k_node_no_parent = UINT8_MAX;

	enum class NodeState : uint8_t
	{
		Invalid,
		Loading, // Node is taken up its space in the world but is not accessable yet
		Loaded, // Fully loaded
		Unloading, // Node is taking up space in the world but is no longer accessible
	};

	// A single node in a spatial world. This is meant to be inherited from for custom data
	struct Node : Nocopy, Nomove
	{
		godot::Vector3i position;
		uint8_t scale_index = 0;

		uint8_t parent_index = k_node_no_parent; // The index we are in our parent
		uint8_t children_mask = 0; // Each bit determines a child [0-7]
		uint8_t neighbour_mask = 0; // Each bit determines a neighbour [0-5]

		NodePtr parent = nullptr; // Octree parent

		NodePtr children[8] = { nullptr }; // Octree children

		NodePtr neighbours[6] = { nullptr }; // Fast access of neighbours of same scale

		robin_hood::unordered_set<entity::Ref> entities;
	};

	struct PartialNode
	{
		NodeState state = NodeState::Invalid;
		uint16_t task_count = 0;
		Clock::time_point last_update_time; // Time since a loader last updated our unload timer
	};

	struct NetworkNode
	{
		uint32_t network_version = 0; // The version of this node. We use this to check if we should update to a newer version if there is one
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
		// We use these to limit operations currently in progress
		std::vector<godot::Vector3i> loading_nodes;
		std::vector<godot::Vector3i> unloading_nodes;
	};

	// A spatial database which has an octree like structure with neighbour pointers and hash maps for each lod. 
	struct World : Nocopy, Nomove
	{
		TypeData* type = nullptr;

		uint8_t max_scale = 0;
		uint8_t node_size = 1;

		bool unloading = false;

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

	struct TypeData
	{
		WorldType world_type;
		ScaleType scale_type;
		NodeType node_type;

		size_t max_scale = k_max_world_scale;
		size_t node_size = 0;

		std::vector<WorldPtr> worlds;
		std::vector<ScalePtr> scales;
	};

	using ScaleCB = cb::Callback<void(ScalePtr)>;
	using NodeCommandCB = cb::Callback<bool(NodePtr)>;

	// Get the scale of a world
	WorldPtr GetWorld(ScalePtr scale);

	// Get a scale in a world given a position
	ScalePtr GetScale(WorldPtr spatial_world, uint8_t scale_index);

	// Get a node in a world at a position and scale
	NodePtr GetNode(WorldPtr world, godot::Vector3i position, uint8_t scale_index);

	// Create a new spatial world given provided types
	WorldPtr CreateWorld(TypeData& type);

	void UnloadWorld(WorldPtr world);

	// Destroy a spatial world
	void DestroyWorld(WorldPtr world);

	bool IsWorldUnloading(WorldPtr world);

	size_t GetNodeCount(WorldPtr world);

	void AddEntity(WorldPtr world, entity::WRef entity);

	void RemoveEntity(WorldPtr world, entity::WRef entity);

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