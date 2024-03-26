#pragma once

#include "FractalCoord.h"

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

    struct FractalNode3D;
	struct FractalRegion3D;
    struct FractalWorld3D;

	struct FractalLoader3D
	{
        FractalWorld3D* world; // World that this camera is loading chunks in
        FractalRegion3D* region; // Region that this camera is in
        FractalCoord3D coord;

        size_t dist_per_lod; // The number of nodes there are until the next lod starts
        size_t min_lod; // The minimum lod this camera can see
        size_t max_lod; // The maximum lod this camera can see
        size_t update_frequency; // The frequency

        std::vector<FractalNode3D*> observed; // Nodes that are being observed by this camera
	};

	struct FractalNode3D
	{
		FractalRegion3D* region; // The region this node is contained in
		FractalCoord3D coord; // The position of this node in its world

		FractalNode3D* parent = nullptr; // Octree parent
		FractalNode3D* children[2][2][2] = { nullptr }; // Octree children
		FractalNode3D* neighbours[6] = { nullptr }; // Fast access of neighbours of same scale

		uint8_t num_observers = 0; // Number of observers looking at me
		Clock::time_point unload_time; // Time when number of observers hit 0. Can be used to unload chunk after an amount of time
		Clock::time_point last_update_time; // Time since last update to help with world merging by testing which chunk was updated most recently
		uint32_t version;
	};

	struct FractalScale3D
	{
		robin_hood::unordered_flat_map<godot::Vector3i, FractalNode3D*, ByteHash<godot::Vector3i>> nodes;

		std::vector<FractalLoader3D*> cameras;
		
		tkrzw::SpinSharedMutex mutex;
	};

	struct FractalRegion3D
	{
		godot::AABB region;

		std::array<FractalScale3D, 16> nodes; // Random access map for each scale

		std::vector<FractalLoader3D*> cameras; // List of chunk cameras this region is focused around

		robin_hood::unordered_flat_set<FractalNode3D*> nodes_changed; // List of nodes marked as updated. Useful for knowing which nodes to update for renderer
		robin_hood::unordered_flat_set<FractalNode3D*> nodes_no_observers; // List of nodes that have no observers observing them
		
		tkrzw::SpinSharedMutex mutex;
	};

	struct FractalWorld3D
	{
		godot::AABB region;

        robin_hood::unordered_flat_map<godot::AABB, FractalRegion3D*> regions;

        std::vector<FractalLoader3D*> cameras; // List of chunk cameras this region is focused around
	};
}