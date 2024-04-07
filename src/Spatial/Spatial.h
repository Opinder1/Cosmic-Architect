#pragma once

#include "SpatialCoord.h"
#include "SpatialAABB.h"

#include "Util/Time.h"
#include "Util/Hash.h"
#include "Util/Nocopy.h"

#include <godot_cpp/variant/vector3.hpp>

#include <robin_hood/robin_hood.h>

#include <flecs/flecs.h>

#include <array>
#include <mutex>
#include <bitset>

namespace voxel_game
{
    constexpr const uint8_t k_max_world_scale = 16;

	// A single node in a spatial world. This is meant to be inherited from for custom data
	struct SpatialNode3D : Nocopy
	{
		uint8_t parent_index = 0; // The index we are in our parent
		uint8_t children_mask = 0; // Each bit determines a child [0-7]
		uint8_t neighbour_mask = 0; // Each bit determines a neighbour [0-5]

		Clock::time_point last_update_time; // Time since a loader last updated our unload timer
		uint32_t num_observers = 0; // Number of observers looking at me (1 for write, more than 1 means shared read)
		uint32_t network_version = 0; // The version of this node. We use this to check if we should update to a newer version if there is one

		SpatialNode3D* parent = nullptr; // Octree parent

		union
		{
			SpatialNode3D* children[2][2][2] = { nullptr }; // Octree children
			SpatialNode3D* children_array[8];
		};

		SpatialNode3D* neighbours[6] = { nullptr }; // Fast access of neighbours of same scale

	};

	// A level of detail map for a world. The world will have multiple of these
	struct SpatialScale3D : Nocopy
	{
		robin_hood::unordered_flat_map<godot::Vector3i, SpatialNode3D*> nodes;
	};

	struct SpatialWorld3D : Nocopy
	{
		SpatialAABB region;

		// Random access map for each scale
		size_t max_scale = k_max_world_scale;
		std::array<SpatialScale3D, k_max_world_scale> scales;
	};

	struct SpatialCommands3D : Nocopy
	{
		// Per thread lists for which the thread adds commands to be resolved later.
		std::vector<godot::Vector3i> nodes_load;
		std::vector<godot::Vector3i> nodes_unload;

		std::vector<godot::Vector3i> nodes_changed; // List of nodes marked as updated. Useful for knowing which nodes to update for renderer
	};
}