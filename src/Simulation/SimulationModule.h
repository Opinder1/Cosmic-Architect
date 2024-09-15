#pragma once

namespace flecs
{
	struct world;
}

namespace voxel_game::sim
{
	struct Module
	{
		Module(flecs::world& world);
	};
}