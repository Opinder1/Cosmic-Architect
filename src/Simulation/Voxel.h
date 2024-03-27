#pragma once

#include "Spatial.h"
#include "PackedOctree.h"

#include <flecs/flecs.h>

#include <bitset>

namespace voxel_game
{
	struct Block
	{
		uint16_t id = 0;
		uint16_t data = 0;
	};

	struct VoxelNode : public SpatialNode3D
	{
		std::vector<flecs::entity_t> entities;

		PackedOctree<flecs::entity_t, 4, 0> block_entities;

		std::vector<flecs::entity_t> factions;

		bool update_children : 1;
		bool modified : 1;
		bool edited : 1;

		Block blocks[16][16][16] = {};
	};

	// Entities that are children of a region entity or world entity should have this component
	// so that the system knows how to place them in the spatial world
	struct VoxelEntityComponent
	{
		SpatialCoord3D coord;
	};

	// Voxel entities can have this component which makes them load components around them
	struct VoxelCameraComponent
	{
		SpatialLoader3D* loader;
	};

	// Entities with this component are region entities and should be a child of a world
	// entity. This entity should have region entities and voxel entities as children.
	struct VoxelRegionComponent
	{
		SpatialRegion3D* region;
	};

	// This component should be added to the world entity. This entity should have region entities
	// as children. The owning entity should also have a region component as a world is a region.
	struct VoxelWorldComponent
	{
		SpatialWorld3D* world;
	};
}