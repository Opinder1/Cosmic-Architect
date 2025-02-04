#pragma once

#include "SpatialCoord.h"

#include "Util/Nocopy.h"
#include "Util/Poly.h"
#include "Util/Util.h"
#include "Util/Hash.h"

#include <godot_cpp/variant/vector3i.hpp>

#include <robin_hood/robin_hood.h>

#include <flecs/flecs.h>

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
	struct Node
	{
		Coord coord;

		NodeState state = NodeState::Unloaded;

		uint8_t parent_index = k_node_no_parent; // The index we are in our parent
		uint8_t children_mask = 0; // Each bit determines a child [0-7]
		uint8_t neighbour_mask = 0; // Each bit determines a neighbour [0-5]

		uint32_t num_observers = 0; // Number of observers looking at me (1 for write, more than 1 means shared read)
		uint32_t network_version = 0; // The version of this node. We use this to check if we should update to a newer version if there is one
		Clock::time_point last_update_time; // Time since a loader last updated our unload timer

		Poly parent = nullptr; // Octree parent

		Poly children[8] = { nullptr }; // Octree children

		Poly neighbours[6] = { nullptr }; // Fast access of neighbours of same scale

		std::vector<flecs::entity_t> entities;
	};

	using NodeMap = robin_hood::unordered_flat_map<godot::Vector3i, Poly>;

	// A level of detail map for a world. The world will have multiple of these
	struct Scale
	{
		uint8_t scale = 0;

		NodeMap nodes;

		// Commands
		std::vector<godot::Vector3i> create_commands;
		std::vector<Poly> load_commands;
		std::vector<Poly> unload_commands;
		std::vector<Poly> destroy_commands;
	};

	using ScaleArray = std::array<Poly, k_max_world_scale>;

	// A spatial database which has an octree like structure with neighbour pointers and hash maps for each lod. 
	struct World : Nocopy, Nomove
	{
		godot::AABB bounds;
		uint8_t max_scale = 1;
		uint8_t node_size = 1;

		Clock::duration node_keepalive = 10s;

		// Scale and Node builder
		PolyType node_type;
		PolyType scale_type;

		PolyEntry<Node> node_entry;
		PolyEntry<Scale> scale_entry;

		// World data
		ScaleArray scales;
	};

	Scale& GetScale(World& spatial_world, Poly scale_poly);

	const Scale& GetScale(const World& spatial_world, Poly scale_poly);

	Scale& GetScale(World& spatial_world, uint8_t scale_index);

	const Scale& GetScale(const World& spatial_world, uint8_t scale_index);

	Node& GetNode(World& spatial_world, Poly node_poly);

	const Node& GetNode(const World& spatial_world, Poly node_poly);

	Poly GetNode(const World& world, Coord coord);
}