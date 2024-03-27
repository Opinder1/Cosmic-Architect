#pragma once

#include "SpatialCoord.h"

#include "Util/Time.h"
#include "Util/ByteHash.h"

#include <godot_cpp/variant/aabb.hpp>
#include <godot_cpp/variant/vector3i.hpp>
#include <godot_cpp/classes/time.hpp>

#include <robin_hood/robin_hood.h>

#include <TKRZW/tkrzw_thread_util.h>

#include <vector>

namespace voxel_game
{
    constexpr const uint8_t k_max_scale = 16;

    struct SpatialNode3D;
	struct SpatialRegion3D;
	struct SpatialScale3D;
    struct SpatialWorld3D;

	struct SpatialLoader3D
	{
        SpatialWorld3D* world; // World that this camera is loading chunks in
        SpatialRegion3D* region; // Region that this camera is in
        SpatialCoord3D coord;

        size_t dist_per_lod; // The number of nodes there are until the next lod starts
        size_t min_lod; // The minimum lod this camera can see
        size_t max_lod; // The maximum lod this camera can see
        size_t update_frequency; // The frequency

        std::vector<SpatialNode3D*> observed; // Nodes that are being observed by this camera
	};

	struct SpatialNode3D
	{
		SpatialWorld3D* world;
		SpatialRegion3D* region; // The region this node is contained in
		SpatialScale3D* scale; // The scale this node is contained in
		SpatialCoord3D coord; // The position of this node in its world

		SpatialNode3D* parent = nullptr; // Octree parent
		SpatialNode3D* children[2][2][2] = { nullptr }; // Octree children
		SpatialNode3D* neighbours[6] = { nullptr }; // Fast access of neighbours of same scale

		Clock::time_point unload_time; // Time when number of observers hit 0. Can be used to unload chunk after an amount of time
		Clock::time_point last_update_time; // Time since last update to help with world merging by testing which chunk was updated most recently
		uint32_t num_observers = 0; // Number of observers looking at me
		uint32_t version;
	};

	struct SpatialScale3D
	{
		robin_hood::unordered_flat_map<godot::Vector3i, SpatialNode3D*, ByteHash<godot::Vector3i>> nodes;

		std::vector<SpatialLoader3D*> cameras;
		
		tkrzw::SpinSharedMutex mutex;
	};

	struct SpatialRegion3D
	{
		godot::AABB region;

		robin_hood::unordered_flat_map<godot::AABB, SpatialRegion3D*> regions;

		std::vector<SpatialLoader3D*> cameras; // List of chunk cameras this region is focused around
		
		tkrzw::SpinSharedMutex mutex;
	};

	struct SpatialWorld3D : SpatialRegion3D
	{
		std::array<SpatialScale3D, 16> scales; // Random access map for each scale

		robin_hood::unordered_flat_set<SpatialNode3D*> nodes_changed; // List of nodes marked as updated. Useful for knowing which nodes to update for renderer
		robin_hood::unordered_flat_set<SpatialNode3D*> nodes_no_observers; // List of nodes that have no observers observing them
	};

	constexpr const size_t coord_size = sizeof(SpatialCoord3D);
	constexpr const size_t loader_size = sizeof(SpatialLoader3D);
	constexpr const size_t node_size = sizeof(SpatialNode3D);
	constexpr const size_t scale_size = sizeof(SpatialScale3D);
	constexpr const size_t region_size = sizeof(SpatialRegion3D);
	constexpr const size_t world_size = sizeof(SpatialWorld3D);
}