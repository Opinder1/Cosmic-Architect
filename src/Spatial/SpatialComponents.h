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

	// An object that tells a spatial world where to load nodes and at what lods
	struct SpatialLoader3DComponent
	{
		SpatialCoord3D coord;

		uint32_t dist_per_lod; // The number of nodes there are until the next lod starts
		uint32_t min_lod; // The minimum lod this camera can see
		uint32_t max_lod; // The maximum lod this camera can see
		uint32_t update_frequency; // The frequency
	};

	// A spatial database which has an octree like structure with neighbour pointers and hash maps for each lod. 
	struct SpatialWorld3DComponent
	{
		SpatialWorld3D* world = nullptr;
	};

	struct SpatialWorld3DCommands : Nocopy
	{
		// Per thread lists for which the thread adds commands to be resolved later.
		std::vector<std::pair<godot::Vector3i, SpatialNode3D*>> nodes_add;
		std::vector<godot::Vector3i> nodes_remove;
		std::vector<godot::Vector3i> nodes_no_loaders; // List of nodes that have no loaders observing them
		std::vector<godot::Vector3i> nodes_changed; // List of nodes marked as updated. Useful for knowing which nodes to update for renderer
	};

	struct SpatialScaleThread3DComponent
	{
		SpatialWorld3D* world = nullptr;
		uint32_t scale = 0;

		SpatialWorld3DCommands commands;
	};

	struct SpatialRegionThread3DComponent : SpatialWorld3DCommands
	{
		SpatialWorld3D* world = nullptr;
		SpatialAABB region;

		std::array<SpatialWorld3DCommands, k_max_world_scale> scale_lists;
	};

	struct SpatialNodeThread3DComponent : SpatialWorld3DCommands
	{
		SpatialWorld3D* world = nullptr;
		SpatialCoord3D node;

		std::array<SpatialWorld3DCommands, k_max_world_scale> scale_lists;
	};
}