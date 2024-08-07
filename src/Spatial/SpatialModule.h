#pragma once

#include "Util/Time.h"

#include <flecs/flecs.h>

namespace voxel_game
{
	struct SpatialCoord3D;
	struct SpatialNode3D;
	struct SpatialWorld3DComponent;

	struct SpatialWorld3DComponent;

	struct SpatialModule
	{
		SpatialModule(flecs::world& world);

		// Add scale workers to a world for each scale the world currently has
		static void AddSpatialScaleWorkers(flecs::entity spatial_world_entity);

		static void RemoveSpatialScaleWorkers(flecs::entity spatial_world_entity);

		static SpatialNode3D* GetNode(const SpatialWorld3DComponent& world, SpatialCoord3D coord);
	};
}