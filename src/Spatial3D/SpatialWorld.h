#pragma once

#include "SpatialCoord.h"

#include "Util/Nocopy.h"
#include "Util/Util.h"
#include "Util/Hash.h"
#include "Util/Poly.h"

#include "Simulation/SimulationComponents.h"

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

	struct Entity
	{
		uint8_t scale = 0;
		godot::Vector3 position;

		uint8_t last_scale = 0;
		godot::Vector3i last_node_pos;
	};

	// An object that tells a spatial world where to load nodes and at what lods
	struct Loader
	{
		uint8_t dist_per_lod = 0; // The number of nodes there are until the next lod starts
		uint8_t min_lod = 0; // The minimum lod this camera can see
		uint8_t max_lod = 0; // The maximum lod this camera can see
		uint8_t update_frequency = 0; // The frequency

		godot::Vector3 position;
	};

	// A single node in a spatial world. This is meant to be inherited from for custom data
	struct Node : Nocopy, Nomove
	{
		Coord coord;

		NodeState state = NodeState::Unloaded;

		uint8_t parent_index = k_node_no_parent; // The index we are in our parent
		uint8_t children_mask = 0; // Each bit determines a child [0-7]
		uint8_t neighbour_mask = 0; // Each bit determines a neighbour [0-5]

		uint32_t num_observers = 0; // Number of observers looking at me (1 for write, more than 1 means shared read)
		uint32_t network_version = 0; // The version of this node. We use this to check if we should update to a newer version if there is one
		Clock::time_point last_update_time; // Time since a loader last updated our unload timer

		Node* parent = nullptr; // Octree parent

		Node* children[8] = { nullptr }; // Octree children

		Node* neighbours[6] = { nullptr }; // Fast access of neighbours of same scale

		robin_hood::unordered_set<Entity*> entities;
	};

	using NodeMap = robin_hood::unordered_map<godot::Vector3i, Node*>;

	// A level of detail map for a world. The world will have multiple of these
	struct Scale : Nocopy, Nomove
	{
		uint8_t index = 0;

		Scale* parent = nullptr;
		Scale* child = nullptr;

		NodeMap nodes;

		robin_hood::unordered_set<Entity*> entities;

		// Commands
		std::vector<godot::Vector3i> create_commands;
		std::vector<Node*> load_commands;
		std::vector<Node*> unload_commands;
		std::vector<Node*> destroy_commands;
	};

	// A spatial database which has an octree like structure with neighbour pointers and hash maps for each lod. 
	struct World : Nocopy, Nomove
	{
		godot::AABB bounds;
		uint8_t max_scale = 1;
		uint8_t node_size = 1;

		Clock::duration node_keepalive = 10s;

		robin_hood::unordered_set<Loader*> loaders;

		robin_hood::unordered_set<Entity*> entities;

		std::array<Scale*, k_max_world_scale> scales;
	};

	using NodeType = PolyType<Node>;
	using ScaleType = PolyType<Scale>;
	using WorldType = PolyType<World>;

	struct Types
	{
		NodeType node_type;
		ScaleType scale_type;
		WorldType world_type;
	};

#define NODE_TO(poly, type) types.node_type.Get<type>(poly)
#define SCALE_TO(poly, type) types.scale_type.Get<type>(poly)
#define WORLD_TO(poly, type) types.world_type.Get<type>(poly)

	const Scale& GetScale(const World& spatial_world, uint8_t scale_index);

	Scale& GetScale(World& spatial_world, uint8_t scale_index);

	const Node* GetNode(const World& world, Coord coord);

	Node* GetNode(World& world, Coord coord);

	void InitializeWorld(World& world);

	void ShutdownWorld(World& world);

	void WorldSetMaxScale(Types& types, World& world, size_t max_scale);

	void WorldCreateNodes(Types& types, World& world, const sim::CFrame& frame);

	void WorldDestroyNodes(Types& types, World& world);

	void ScaleLoadNodes(const World& world, Scale& scale, const sim::CFrame& frame);

	void ScaleUnloadNodes(const World& world, Scale& scale, const sim::CFrame& frame);

	void WorldUpdateEntityScales(World& world);

	void ScaleUpdateEntityNodes(const World& world, Scale& scale);
}