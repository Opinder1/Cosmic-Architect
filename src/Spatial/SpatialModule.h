#pragma once

#include "SpatialCoord.h"
#include "SpatialAABB.h"

namespace flecs
{
	struct world;
	struct entity;
}

namespace voxel_game
{
	struct SpatialLoader3D;
	struct SpatialRegion3D;
	struct SpatialWorld3D;

	struct SpatialEntityTag {};

	// Entities that are children of a world entity should have this component
	// so that the system knows how to place them in the spatial world
	struct SpatialPositionComponent
	{
		SpatialCoord3D coord;
	};

	struct SpatialBoxComponent
	{
		SpatialAABB aabb;
	};

	struct SpatialSphereComponent
	{
		double radius;
	};

	// Spatial world entities can have this component which makes them load nodes around them
	struct SpatialCameraComponent
	{
		SpatialLoader3D* loader;
	};

	// This component should be added to the world entity. This entity should have region entities
	// as children. The owning entity should also have a region component as a world is a region.
	struct SpatialWorldComponent
	{
		SpatialWorld3D* world;
	};

	struct SpatialWorldThreadComponent
	{
		SpatialWorld3D* world;
		uint32_t id;
	};

	struct SpatialModule
	{
		SpatialModule(flecs::world& world);

		static void ProcessThread(flecs::entity entity, SpatialWorldThreadComponent& world_thread);

		static void ProcessWorld(flecs::entity entity, SpatialWorldComponent& spatial_world);
	};
}