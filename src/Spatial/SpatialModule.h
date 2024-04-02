#pragma once

namespace flecs
{
	struct world;
}

namespace voxel_game
{
	struct SpatialNode3D;
	struct SpatialWorld3DComponent;
	struct SpatialCoord3D;

	struct SpatialModule
	{
		SpatialModule(flecs::world& world);

		static SpatialNode3D* GetNode(const SpatialWorld3DComponent& world, SpatialCoord3D coord);
	};
}