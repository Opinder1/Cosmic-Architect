#pragma once

namespace flecs
{
	struct world;
}

namespace voxel_game
{
	struct GalaxyModule
	{
		GalaxyModule(flecs::world& world);
	};
}