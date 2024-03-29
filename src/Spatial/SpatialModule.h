#pragma once

namespace flecs
{
	struct world;
}

namespace voxel_game
{
	struct SpatialLoader3D;
	struct SpatialRegion3D;
	struct SpatialWorld3D;

	// Entities that are children of a world entity should have this component
	// so that the system knows how to place them in the spatial world
	struct SpatialEntityComponent
	{
		SpatialCoord3D coord;
	};

	// Spatial world entities can have this component which makes them load nodes around them
	struct SpatialCameraComponent
	{
		SpatialLoader3D* loader;
	};

	struct SpatialWorldThreadComponent
	{
		SpatialWorld3D* world;
		uint32_t id;
	};

	// This component should be added to the world entity. This entity should have region entities
	// as children. The owning entity should also have a region component as a world is a region.
	struct SpatialWorldComponent
	{
		SpatialWorld3D* world;
	};

	struct SpatialModule
	{
		SpatialModule(flecs::world& world);
	};
}