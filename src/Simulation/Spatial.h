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

	// An object that tells a spatial world where to load nodes and at what lods
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

	// A single node in a spatial world. This is meant to be inherited from for custom data
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

	// A level of detail map for a world. The world will have multiple of these
	struct SpatialScale3D
	{
		robin_hood::unordered_flat_map<godot::Vector3i, SpatialNode3D*, ByteHash<godot::Vector3i>> nodes;

		std::vector<SpatialLoader3D*> loaders; // List of loaders that can see this scale
		
		// Each scale has a mutex which protects adding and removing nodes from the tree and hash map
		// When adding and removing from multiple scales at once the world mutex should be locked
		tkrzw::SpinSharedMutex mutex;
	};

	// A region in a world. Can also be a region within a region. Is used so that threads can modify different bits of the world in parallel
	struct SpatialRegion3D
	{
		godot::AABB region;

		// If this region has any child regions then they must be locked as well or add a new one to access nodes in this region
		robin_hood::unordered_flat_map<godot::AABB, SpatialRegion3D*> regions;

		std::vector<SpatialLoader3D*> loaders; // List of chunk cameras this region is focused around
		
		// Each region has a mutex so different threads can read and write to nodes from different regions simultaneously
		// The world also uses this mutex to protect its members as well
		tkrzw::SpinSharedMutex mutex;
	};

	// A spatial database which has an octree like structure with neighbour pointers and hash maps for each lod. 
	struct SpatialWorld3D : SpatialRegion3D
	{
		std::array<SpatialScale3D, 16> scales; // Random access map for each scale

		robin_hood::unordered_flat_set<SpatialNode3D*> nodes_changed; // List of nodes marked as updated. Useful for knowing which nodes to update for renderer
		robin_hood::unordered_flat_set<SpatialNode3D*> nodes_no_observers; // List of nodes that have no observers observing them
	};
}