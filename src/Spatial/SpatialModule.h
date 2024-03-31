#pragma once

#include "SpatialCoord.h"
#include "SpatialAABB.h"

#include <godot_cpp/variant/vector3.hpp>

namespace flecs
{
	struct world;
	struct entity;
}

namespace voxel_game
{
	struct SpatialLoader3D;
	struct SpatialScale3D;
	struct SpatialWorld3D;
	struct SpatialWorld3DThread;

	struct SpatialEntityTag {};

	// Entities that are children of a world entity should have this component
	// so that the system knows how to place them in the spatial world
	struct SpatialPositionComponent
	{
		godot::Vector3 position;
		uint32_t scale;
	};

	struct SpatialBoxComponent
	{
		godot::Vector3 size;
	};

	struct SpatialSphereComponent
	{
		double radius = 0.0;
	};

	// Spatial world entities can have this component which makes them load nodes around them
	struct SpatialCameraComponent
	{
		SpatialLoader3D* loader = nullptr;
	};

	// This component should be added to the world entity. This entity should have region entities
	// as children. The owning entity should also have a region component as a world is a region.
	struct SpatialWorldComponent
	{
		SpatialWorld3D* world = nullptr;
	};

	struct SpatialWorldRefComponent
	{
		SpatialWorld3D* world = nullptr;
	};

	struct SpatialWorldThreadComponent
	{
		SpatialWorld3D* world = nullptr;
		uint32_t id = UINT32_MAX;
	};

	struct SpatialModule
	{
		SpatialModule(flecs::world& world);

		static void ProgressWorldThread(flecs::entity entity, SpatialWorldThreadComponent& world_thread);

		static void ProgressWorld(flecs::entity entity, SpatialWorldComponent& spatial_world);
	};
}