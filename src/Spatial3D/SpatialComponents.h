#pragma once

#include "SpatialWorld.h"

#include "Physics3D/PhysicsComponents.h"

#include "Util/Util.h"
#include "Util/Nocopy.h"
#include "Util/Debug.h"

#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/vector3i.hpp>
#include <godot_cpp/variant/aabb.hpp>

#include <robin_hood/robin_hood.h>

#include <array>
#include <vector>

namespace voxel_game::spatial3d
{
	// An object that tells a spatial world where to load nodes and at what lods
	struct CLoader
	{
		uint8_t dist_per_lod = 0; // The number of nodes there are until the next lod starts
		uint8_t min_lod = 0; // The minimum lod this camera can see
		uint8_t max_lod = 0; // The maximum lod this camera can see
		uint8_t update_frequency = 0; // The frequency
	};

	// Add this component to a child of a scale marker to signify it represents a region in that world
	struct CRegion
	{
		godot::AABB aabb;
	};

	// An entity which is in a spatial world. It will be given a node its part of and loaded/unloaded with that node
	struct CEntity
	{
		WorldPtr world;
		uint8_t scale = 0;
		godot::Vector3 position;

		uint8_t last_scale = 0;
		godot::Vector3i last_node_pos;
	};

	// A spatial database which has an octree like structure with neighbour pointers and hash maps for each lod. 
	struct CWorld
	{
		WorldPtr world = nullptr;
	};
}