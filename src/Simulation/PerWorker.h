#pragma once

namespace flecs
{
	struct world;
}

namespace voxel_game
{
	// Per worker example module that runs code for each worker thread of the world
	struct PerWorkerModule
	{
		PerWorkerModule(flecs::world& world);
	};
}