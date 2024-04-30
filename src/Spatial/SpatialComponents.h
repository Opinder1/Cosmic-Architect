#pragma once

#include "Spatial.h"

#include "Util/Nocopy.h"

#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/vector3i.hpp>

#include <vector>

namespace flecs
{
	struct world;
}

namespace voxel_game
{
	// Phases which are used to synchronise the ecs between running each thread type in parallel
	struct WorldLoaderProgressPhase {};
	struct WorldRegionProgressPhase {};
	struct WorldNodeProgressPhase {};
	struct WorldScaleProgressPhase {};
	struct WorldProgressPhase {};

	// Add this component to a child of an entity to specify the worker wants to access the world in parallel
	struct ParallelWorkerComponent {}; // TODO: Move out of the spatial module ideally

	// Specify that this entity is within a spatial world (the world is the entities parent)
	struct SpatialEntity3DComponent {};

	// Per thread lists for which the thread adds commands to be resolved later.
	struct SpatialScaleCommands
	{
		std::vector<godot::Vector3i> nodes_load;
		std::vector<godot::Vector3i> nodes_unload;
	};

	// Add this component to a child of a spatial world to specify that it will add commands that will execute on the world
	struct SpatialCommands3DComponent
	{
		SpatialScaleCommands scales[k_max_world_scale];
	};

	// Define which scale an entity is within
	struct SpatialScale3DComponent
	{
		uint8_t scale = 0;
	};

	// Add this component to a child of a spatial world to signify it represents a region in that world
	struct SpatialRegion3DComponent
	{
		SpatialAABB region;
	};

	// Add this component to a child of a spatial world to signify it represents a node in that world
	struct SpatialNode3DComponent
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

	// A spatial database which has an octree like structure with neighbour pointers and hash maps for each lod. 
	struct SpatialWorld3DComponent
	{
		SpatialWorld3D world;

		flecs::query<SpatialCommands3DComponent> commands_query;
	};

	struct SpatialComponents
	{
		SpatialComponents(flecs::world& world);
	};
}