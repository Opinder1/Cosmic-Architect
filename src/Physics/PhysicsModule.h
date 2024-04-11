#pragma once

namespace flecs
{
	struct world;
}

namespace voxel_game
{
	struct PhysicsModule
	{
		PhysicsModule(flecs::world& world);
	};
}