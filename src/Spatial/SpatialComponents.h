#pragma once

#include "Spatial.h"

#include "Util/Nocopy.h"

#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/vector3i.hpp>

#include <vector>

namespace voxel_game
{
	struct SpatialNode3D;
	struct SpatialWorld3D;

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

	struct SpatialCommands3DComponent : Nocopy
	{
		std::array<SpatialCommands3D, k_max_world_scale> scales;
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

	struct SpatialScaleThread3DComponent
	{
		uint32_t scale = 0;
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
}