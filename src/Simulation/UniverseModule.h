#pragma once

namespace flecs
{
	struct world;
}

namespace voxel_game
{
	struct UniverseModule
	{
		UniverseModule(flecs::world& world);
	};
}