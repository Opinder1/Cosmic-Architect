#pragma once

#include "SpatialCoord.h"
#include "SpatialAABB.h"

#include "Util/Time.h"
#include "Util/ByteHash.h"

#include <godot_cpp/variant/vector3i.hpp>

#include <robin_hood/robin_hood.h>

#include <TKRZW/tkrzw_thread_util.h>

#include <vector>
#include <array>
#include <bitset>

namespace voxel_game
{
    constexpr const uint8_t k_max_world_scale = 16;

    struct SpatialNode3D;
    struct SpatialWorld3D;

	// An object that tells a spatial world where to load nodes and at what lods
	struct SpatialLoader3D
	{
        SpatialCoord3D coord;

        uint32_t dist_per_lod; // The number of nodes there are until the next lod starts
		uint32_t min_lod; // The minimum lod this camera can see
		uint32_t max_lod; // The maximum lod this camera can see
		uint32_t update_frequency; // The frequency
	};

	// A single node in a spatial world. This is meant to be inherited from for custom data
	struct SpatialNode3D
	{
		SpatialNode3D* parent = nullptr; // Octree parent
		uint8_t pos_in_parent = 0;

		SpatialNode3D* children[2][2][2] = { nullptr }; // Octree children
		SpatialNode3D* neighbours[6] = { nullptr }; // Fast access of neighbours of same scale

		Clock::time_point unload_time; // Time when number of observers hit 0. Can be used to unload chunk after an amount of time
		Clock::time_point last_update_time; // Time since last update to help with world merging by testing which chunk was updated most recently
		uint32_t num_observers = 0; // Number of observers looking at me
		uint32_t version = 0; // The version of this node. We use this to check if we should update to a newer version if there is one

	};

	struct SpatialWorld3DThread
	{
		// The AABB that the thread will be accessing
		SpatialAABB aabb;

		// Loaders that this thread should update
		std::vector<SpatialLoader3D*> loaders_to_update;

		struct Lists
		{
			// Per thread lists for which the thread adds commands to be resolved later.
			std::vector<std::pair<godot::Vector3i, SpatialNode3D*>> nodes_add;
			std::vector<godot::Vector3i> nodes_remove;
			std::vector<godot::Vector3i> nodes_no_loaders; // List of nodes that have no loaders observing them
			std::vector<godot::Vector3i> nodes_changed; // List of nodes marked as updated. Useful for knowing which nodes to update for renderer
		};

		std::array<Lists, k_max_world_scale> scale_lists;
	};

	// A level of detail map for a world. The world will have multiple of these
	using SpatialScale3D = robin_hood::unordered_flat_map<godot::Vector3i, SpatialNode3D*, ByteHash<godot::Vector3i>>;

	// A spatial database which has an octree like structure with neighbour pointers and hash maps for each lod. 
	struct SpatialWorld3D
	{
		SpatialAABB aabb;

		// Random access map for each scale
		std::array<SpatialScale3D, k_max_world_scale> scales;

		// List of loaders that can see this world
		std::vector<SpatialLoader3D*> loaders;

		// Per thread data for updating the world in parallel
		robin_hood::unordered_node_map<uint64_t, SpatialWorld3DThread> threads;
	};

	SpatialWorld3D* CreateNewWorld();

	void DestroyWorld(SpatialWorld3D* world);

	SpatialNode3D* GetNode(SpatialWorld3D* world, SpatialCoord3D coord);
}