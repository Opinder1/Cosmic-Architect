#pragma once

#include "SpatialCoord.h"
#include "SpatialAABB.h"

#include "Util/Time.h"
#include "Util/Hash.h"
#include "Util/Nocopy.h"
#include "Util/Callback.h"

#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/vector3i.hpp>

#include <robin_hood/robin_hood.h>

#include <flecs/flecs.h>

#include <vector>
#include <array>
#include <bitset>

namespace flecs
{
	struct world;
}

namespace voxel_game
{
	struct SpatialWorld3DComponent;

	constexpr const uint8_t k_max_world_scale = 16;

	// Phases which are used to synchronise the ecs between running each thread type in parallel
	struct WorldLoaderWorkerPhase {}; // In this phase we process all loaders of all worlds in parallel
	struct WorldRegionWorkerPhase {}; // In this phase we process select regions from different worlds in parallel
	struct WorldNodeWorkerPhase {}; // In this phase we process select nodes from different worlds in parallel
	struct WorldScaleWorkerPhase {}; // In this phase we process all scales of all worlds in parallel
	struct WorldWorkerPhase {}; // In this phase we process all worlds in parallel

	// Specify that this entity is within a spatial world (the world is the entities parent)
	struct SpatialEntity3DComponent {};

	// Define which scale an entity is within
	struct SpatialScale3DWorkerComponent
	{
		uint8_t scale = 0;
	};

	// Add this component to a child of a spatial world to signify it represents a region in that world
	struct SpatialRegion3DWorkerComponent
	{
		SpatialAABB region;
	};

	// Add this component to a child of a spatial world to signify it represents a node in that world
	struct SpatialNode3DWorkerComponent
	{
		SpatialCoord3D node;
	};

	// An object that tells a spatial world where to load nodes and at what lods
	struct SpatialLoader3DComponent
	{
		SpatialCoord3D coord;

		uint8_t dist_per_lod = 0; // The number of nodes there are until the next lod starts
		uint8_t min_lod = 0; // The minimum lod this camera can see
		uint8_t max_lod = 0; // The maximum lod this camera can see
		uint8_t update_frequency = 0; // The frequency
	};

	// A single node in a spatial world. This is meant to be inherited from for custom data
	struct SpatialNode3D : Nocopy
	{
		SpatialCoord3D coord;

		uint8_t parent_index = 0; // The index we are in our parent
		uint8_t children_mask = 0; // Each bit determines a child [0-7]
		uint8_t neighbour_mask = 0; // Each bit determines a neighbour [0-5]

		uint32_t num_observers = 0; // Number of observers looking at me (1 for write, more than 1 means shared read)
		uint32_t network_version = 0; // The version of this node. We use this to check if we should update to a newer version if there is one
		Clock::time_point last_update_time; // Time since a loader last updated our unload timer

		SpatialNode3D* parent = nullptr; // Octree parent

		union
		{
			SpatialNode3D* children[2][2][2] = { nullptr }; // Octree children
			SpatialNode3D* children_array[8];
		};

		SpatialNode3D* neighbours[6] = { nullptr }; // Fast access of neighbours of same scale
	};

	using SpatialScaleNodeCommands = std::vector<godot::Vector3i>;

	using SpatialNodeCreateCB = cb::Callback<std::unique_ptr<SpatialNode3D>()>;
	using SpatialNodeDestroyCB = cb::Callback<void(std::unique_ptr<SpatialNode3D>&)>;
	using SpatialNodeProcessCB = cb::Callback<void(SpatialNode3D&)>;

	// A level of detail map for a world. The world will have multiple of these
	struct SpatialScale3D : Nocopy
	{
		robin_hood::unordered_flat_map<godot::Vector3i, std::unique_ptr<SpatialNode3D>> nodes;

		SpatialScaleNodeCommands load_commands;
		SpatialScaleNodeCommands unload_commands;
		SpatialScaleNodeCommands tick_commands;
	};

	// A spatial database which has an octree like structure with neighbour pointers and hash maps for each lod. 
	struct SpatialWorld3DComponent : Nocopy
	{
		SpatialAABB bounds;

		// Random access map for each scale
		size_t max_scale = k_max_world_scale;
		std::array<SpatialScale3D, k_max_world_scale> scales;

		flecs::query_t* loaders_query = nullptr;

		SpatialNodeCreateCB create_node;
		SpatialNodeDestroyCB destroy_node;

		std::vector<SpatialNodeProcessCB> load_command_processors;
		std::vector<SpatialNodeProcessCB> unload_command_processors;
		std::vector<SpatialNodeProcessCB> tick_command_processors;
	};

	struct SpatialComponents
	{
		SpatialComponents(flecs::world& world);
	};
}