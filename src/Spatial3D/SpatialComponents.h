#pragma once

#include "SpatialCoord.h"
#include "SpatialAABB.h"

#include "Physics3D/PhysicsComponents.h"

#include "Util/Poly.h"
#include "Util/Time.h"
#include "Util/Hash.h"
#include "Util/Nocopy.h"
#include "Util/Callback.h"
#include "Util/Debug.h"

#include <godot_cpp/variant/vector3i.hpp>

#include <robin_hood/robin_hood.h>

#include <flecs/flecs.h>

#include <array>
#include <vector>

namespace flecs
{
	struct world;
}

namespace voxel_game::spatial3d
{
	struct Components
	{
		Components(flecs::world& world);
	};

	struct Node;
	struct Scale;

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

	struct NodeCreatePhase {}; // In this phase we create any new nodes
	struct NodeLoadPhase {}; // In this phase we load any new nodes
	struct NodeUnloadPhase {}; // In this phase we unload any marked nodes
	struct NodeDestroyPhase {}; // In this phase we destroy and unloaded nodes

	struct WorldCreateEvent {};
	struct WorldDestroyEvent {};

	// Specify that this entity is within a spatial world (the world is the entities parent)
	struct Entity
	{
		Coord node;
	};

	// An object that tells a spatial world where to load nodes and at what lods
	struct Loader
	{
		uint8_t dist_per_lod = 0; // The number of nodes there are until the next lod starts
		uint8_t min_lod = 0; // The minimum lod this camera can see
		uint8_t max_lod = 0; // The maximum lod this camera can see
		uint8_t update_frequency = 0; // The frequency
	};

	// Add this component to a child of a spatial world to signify it represents a scale of that world
	struct ScaleMarker
	{
		uint8_t scale = 0;
	};

	// Add this component to a child of a spatial world to signify it represents a region in that world
	struct RegionMarker
	{
		AABB region;
	};

	// A single node in a spatial world. This is meant to be inherited from for custom data
	struct Node : Nocopy
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

		union
		{
			Node* children[2][2][2] = { nullptr }; // Octree children
			Node* children_array[8];
		};

		Node* neighbours[6] = { nullptr }; // Fast access of neighbours of same scale
	};

	using NodeMap = robin_hood::unordered_flat_map<godot::Vector3i, Poly>;

	// A level of detail map for a world. The world will have multiple of these
	struct Scale : Nocopy
	{
		NodeMap nodes;

		// Commands
		std::vector<godot::Vector3i> create_commands;
		std::vector<Poly> load_commands;
		std::vector<Poly> unload_commands;
		std::vector<Poly> destroy_commands;
	};

	using WorldScaleArray = std::array<Scale, k_max_world_scale>;

	// A spatial database which has an octree like structure with neighbour pointers and hash maps for each lod. 
	struct World : Nocopy
	{
		AABB bounds;
		uint8_t max_scale = 1;
		uint8_t node_size = 1;

		Clock::duration node_keepalive = 10s;

		// Scale and Node builder
		PolyType node_type;

		PolyEntry<Node> node_entry;

		// Queries
		flecs::query<const Entity> entities_query;
		flecs::query<const Loader, const physics3d::Position> loaders_query;
		flecs::query<const ScaleMarker> scale_marker_query;
		flecs::query<const RegionMarker> region_marker_query;

		// World data
		WorldScaleArray scales;
	};
}