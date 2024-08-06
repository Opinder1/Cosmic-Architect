#pragma once

#include "SpatialCoord.h"
#include "SpatialAABB.h"
#include "SpatialCommands.h"

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

namespace voxel_game
{
	struct SpatialNode3D;
	struct SpatialScale3D;
	struct SpatialNodeCommandProcessorBase;

	// The max scale that a world can have
	constexpr const uint8_t k_max_world_scale = 16;
	constexpr const size_t k_max_frame_load_commands = 16;
	constexpr const size_t k_max_frame_unload_commands = 16;
	constexpr const uint8_t k_node_no_parent = UINT8_MAX;

	enum class NodeState : uint8_t
	{
		Unloaded, // Created but not loaded
		Loaded, // Fully loaded

		Loading, // Has a load command
		Unloading, // Has a unload command
		Deleting, // Has a delete command
	};

	using SpatialScaleNodeCommands = std::vector<godot::Vector3i>;
	using SpatialNodeMap = robin_hood::unordered_flat_map<godot::Vector3i, std::unique_ptr<SpatialNode3D>>;
	using SpatialNodeCommandProcessors = std::vector<SpatialNodeCommandProcessorBase>;
	using WorldScaleArray = std::array<std::unique_ptr<SpatialScale3D>, k_max_world_scale>;

	// Phases which are used to synchronise the ecs between running each thread type in parallel
	struct SpatialWorldMultithreadPhase {};

	struct WorldRegionWorkerPhase {}; // In this phase we process select regions from different worlds in parallel
	struct WorldScaleWorkerPhase {}; // In this phase we process all scales of all worlds in parallel
	struct WorldWorkerPhase {}; // In this phase we process all worlds in parallel
	struct WorldCreatePhase {}; // In this phase we create any new nodes
	struct WorldLoadPhase {}; // In this phase we load any new nodes
	struct WorldUnloadPhase {}; // In this phase we unload any marked nodes
	struct WorldDestroyPhase {}; // In this phase we destroy and unloaded nodes
	struct WorldEndPhase {}; // In this phase we can do any singlethreaded post processing

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

		NodeState state = NodeState::Unloaded;

		uint8_t parent_index = k_node_no_parent; // The index we are in our parent
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

	// A level of detail map for a world. The world will have multiple of these
	struct SpatialScale3D : Nocopy
	{
		SpatialNodeMap nodes;

		// Commands
		SpatialScaleNodeCommands create_commands;
		SpatialScaleNodeCommands load_commands;
		SpatialScaleNodeCommands unload_commands;
		SpatialScaleNodeCommands destroy_commands;
	};

	// A spatial database which has an octree like structure with neighbour pointers and hash maps for each lod. 
	struct SpatialWorld3DComponent : Nocopy
	{
		SpatialAABB bounds;
		bool initialized = false;
		uint8_t max_scale = 1;
		uint8_t node_size = 1;

		// Queries
		const flecs::query_t* entities_query = nullptr;
		const flecs::query_t* scale_workers_query = nullptr;
		const flecs::query_t* region_workers_query = nullptr;
		const flecs::query_t* loaders_query = nullptr;

		// World data
		WorldScaleArray scales;

		// Scale and Node builder
		SpatialBuilderBase builder = SpatialBuilder<SpatialScale3D, SpatialNode3D>();

		// Command processors
		SpatialNodeCommandProcessors load_command_processors;
		SpatialNodeCommandProcessors unload_command_processors;
		SpatialNodeCommandProcessors tick_command_processors;
	};

	struct SpatialComponents
	{
		SpatialComponents(flecs::world& world);
	};
}