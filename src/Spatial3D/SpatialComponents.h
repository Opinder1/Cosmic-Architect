#pragma once

#include "SpatialWorld.h"

#include "Physics3D/PhysicsComponents.h"

#include "Util/Util.h"
#include "Util/Nocopy.h"
#include "Util/Debug.h"

#include <godot_cpp/variant/vector3i.hpp>
#include <godot_cpp/variant/aabb.hpp>

#include <robin_hood/robin_hood.h>

#include <flecs/flecs.h>

#include <array>
#include <vector>

namespace voxel_game::spatial3d
{
	struct Components
	{
		Components(flecs::world& world);
	};

	struct PNodeCreate {}; // In this phase we create any new nodes
	struct PNodeLoad {}; // In this phase we load any new nodes
	struct PNodeUnload {}; // In this phase we unload any marked nodes
	struct PNodeDestroy {}; // In this phase we destroy and unloaded nodes

	struct EWorldCreate {};
	struct EWorldDestroy {};

	// An object that tells a spatial world where to load nodes and at what lods
	struct CLoader
	{
		Loader* loader = nullptr;
	};

	// Add this component to a child of a scale marker to signify it represents a region in that world
	struct CRegion
	{
		godot::AABB aabb;
	};

	// An entity which is in a spatial world. It will be given a node its part of and loaded/unloaded with that node
	struct CEntity
	{
		Entity* entity = nullptr;
	};

	// A level of detail map for a world. The world will have multiple of these
	struct CScale
	{
		Scale* scale = nullptr;
	};

	// A spatial database which has an octree like structure with neighbour pointers and hash maps for each lod. 
	struct CWorld
	{
		World* world = nullptr;

		Types types;
	};
}