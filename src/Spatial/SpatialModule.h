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

		static void AddSpatialScaleWorkers(flecs::world& world, flecs::entity_t spatial_world_entity);

		static SpatialNode3D* GetNode(const SpatialWorld3DComponent& world, SpatialCoord3D coord);
	};
}