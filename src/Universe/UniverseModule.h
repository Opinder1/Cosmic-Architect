#pragma once

namespace flecs
{
	struct world;
}

namespace voxel_game::universe
{
	struct Module
	{
		Module(flecs::world& world);
	};
}