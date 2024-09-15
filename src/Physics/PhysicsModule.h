#pragma once

namespace flecs
{
	struct world;
}

namespace voxel_game::physics
{
	struct Module
	{
		Module(flecs::world& world);
	};
}