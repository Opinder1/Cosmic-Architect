#pragma once

#include "SpatialCoord.h"
#include "SpatialAABB.h"

#include "Util/Time.h"
#include "Util/ByteHash.h"
#include "Util/Nocopy.h"

#include <godot_cpp/variant/vector3i.hpp>
#include <godot_cpp/variant/vector3.hpp>

#include <robin_hood/robin_hood.h>

#include <flecs/flecs.h>

#include <vector>
#include <array>

namespace voxel_game
{
    constexpr const uint8_t k_max_world_scale = 16;

	struct SpatialEntity3DTag {};

	// Entities that are children of a world entity should have this component
	// so that the system knows how to place them in the spatial world
	struct SpatialPosition3DComponent
	{
		godot::Vector3 position;
		uint32_t scale;
	};

	struct SpatialBox3DComponent
	{
		godot::Vector3 size;
	};

	struct SpatialSphere3DComponent
	{
		double radius = 0.0;
	};

	// An object that tells a spatial world where to load nodes and at what lods
	struct SpatialLoader3DComponent
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

	// A level of detail map for a world. The world will have multiple of these
	struct SpatialScale3D
	{
		robin_hood::unordered_flat_map<godot::Vector3i, SpatialNode3D*, ByteHash<godot::Vector3i>> nodes;
	};

	// A spatial database which has an octree like structure with neighbour pointers and hash maps for each lod. 
	struct SpatialWorld3DComponent : Nocopy
	{
		SpatialAABB region;

		robin_hood::unordered_flat_map<flecs::entity_t, godot::Vector3> loaders;

		robin_hood::unordered_flat_map<size_t, flecs::entity_t> command_lists;

		// Random access map for each scale
		std::array<SpatialScale3D, k_max_world_scale> scales;
	};

	struct SpatialWorld3DThreadComponent : Nocopy
	{
		uint32_t min_scale;
		uint32_t max_scale;

		SpatialAABB region;
	};

	struct SpatialWorld3DCommands : Nocopy
	{
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
}