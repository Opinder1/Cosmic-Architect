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
	struct SpatialNode3D;
	struct SpatialWorld3D;

	struct WorldLoaderProgressPhase {};
	struct WorldRegionProgressPhase {};
	struct WorldNodeProgressPhase {};
	struct WorldScaleProgressPhase {};
	struct WorldProgressPhase {};

	struct SpatialEntity3DComponent {};

	struct SpatialScale3DComponent
	{
		uint8_t scale;
	};

	struct SpatialCommands3DComponent
	{
		std::array<SpatialCommands3D, k_max_world_scale> scales;
	};

	// An object that tells a spatial world where to load nodes and at what lods
	struct SpatialLoader3DComponent
	{
		SpatialCoord3D coord;

		uint8_t dist_per_lod; // The number of nodes there are until the next lod starts
		uint8_t min_lod; // The minimum lod this camera can see
		uint8_t max_lod; // The maximum lod this camera can see
		uint8_t update_frequency; // The frequency
	};

	struct SpatialScaleThread3DComponent
	{
		uint8_t scale;
	};

	struct SpatialRegionThread3DComponent
	{
		SpatialAABB region;
	};

	struct SpatialNodeThread3DComponent
	{
		SpatialCoord3D node;
	};

	// A spatial database which has an octree like structure with neighbour pointers and hash maps for each lod. 
	struct SpatialWorld3DComponent
	{
		SpatialWorld3D* world = nullptr;

		flecs::query<SpatialCommands3DComponent> commands_query;
	};

	struct SpatialComponents
	{
		SpatialComponents(flecs::world& world);
	};
}