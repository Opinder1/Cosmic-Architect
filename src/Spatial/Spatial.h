#pragma once

#include "SpatialCoord.h"
#include "SpatialAABB.h"

#include "Util/Time.h"
#include "Util/ByteHash.h"
#include "Util/Nocopy.h"

#include <godot_cpp/variant/vector3.hpp>

#include <robin_hood/robin_hood.h>

#include <flecs/flecs.h>

#include <array>
#include <mutex>

namespace voxel_game
{
    constexpr const uint8_t k_max_world_scale = 16;

	// A single node in a spatial world. This is meant to be inherited from for custom data
	struct SpatialNode3D : Nocopy
	{
		SpatialNode3D* parent = nullptr; // Octree parent
		uint8_t pos_in_parent = 0;

		union
		{
			SpatialNode3D* children[2][2][2] = { nullptr }; // Octree children
			SpatialNode3D* children_array[8];
		};
		SpatialNode3D* neighbours[6] = { nullptr }; // Fast access of neighbours of same scale

		Clock::time_point last_update_time; // Time since a loader last updated our unload timer
		uint32_t num_observers = 0; // Number of observers looking at me (1 for write, more than 1 means shared read)
		uint32_t network_version = 0; // The version of this node. We use this to check if we should update to a newer version if there is one

	};

	// A level of detail map for a world. The world will have multiple of these
	struct SpatialScale3D : Nocopy
	{
		robin_hood::unordered_flat_map<godot::Vector3i, SpatialNode3D*, ByteHash<godot::Vector3i>> nodes;
	};

	struct SpatialWorld3D : Nocopy
	{
		SpatialAABB region;

		robin_hood::unordered_flat_map<flecs::entity_t, godot::Vector3> loaders;

		// Random access map for each scale
		size_t max_scale = k_max_world_scale;
		std::array<SpatialScale3D, k_max_world_scale> scales;
	};
}