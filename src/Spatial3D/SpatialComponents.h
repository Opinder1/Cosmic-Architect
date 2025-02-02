#pragma once

#include "SpatialWorld.h"
#include "SpatialCoord.h"

#include "Physics3D/PhysicsComponents.h"

#include "Util/Util.h"
#include "Util/Poly.h"
#include "Util/Nocopy.h"
#include "Util/Debug.h"

#include <godot_cpp/variant/vector3i.hpp>

#include <flecs/flecs.h>

#include <array>
#include <vector>

namespace voxel_game::spatial3d
{
	struct Components
	{
		Components(flecs::world& world);
	};

	struct NodeCreatePhase {}; // In this phase we create any new nodes
	struct NodeLoadPhase {}; // In this phase we load any new nodes
	struct NodeUnloadPhase {}; // In this phase we unload any marked nodes
	struct NodeDestroyPhase {}; // In this phase we destroy and unloaded nodes

	struct WorldCreateEvent {};
	struct WorldDestroyEvent {};

	// An object that tells a spatial world where to load nodes and at what lods
	struct Loader
	{
		uint8_t dist_per_lod = 0; // The number of nodes there are until the next lod starts
		uint8_t min_lod = 0; // The minimum lod this camera can see
		uint8_t max_lod = 0; // The maximum lod this camera can see
		uint8_t update_frequency = 0; // The frequency
	};

	struct Entity
	{
		godot::Vector3i node_pos;
	};

	// Specify the scale that the entity is at
	struct RScale {};

	// Add this component to a child of a spatial world to signify it represents a scale of that world
	struct ScaleMarker
	{
		uint8_t scale = 0;

		flecs::query<Entity, const physics3d::Position> update_parent_node_query;
	};

	// Add this component to a child of a scale marker to signify it represents a region in that world
	struct RegionMarker
	{
		godot::AABB aabb;
	};

	// Add this component to a child of a scale marker to signify it represents a node in that world
	struct NodeMarker
	{
		godot::Vector3i node_pos;
	};

	struct WorldMarker
	{
		World world;

		// Queries
		flecs::query<const Loader, const physics3d::Position> loaders_query;
		flecs::query<const ScaleMarker> scale_marker_query;
		flecs::query<const RegionMarker> region_marker_query;
		flecs::query<const NodeMarker> node_marker_query;
	};
}