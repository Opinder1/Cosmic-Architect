#pragma once

namespace flecs
{
	struct world;
}

namespace voxel_game::physics3d
{
	struct Module
	{
		Module(flecs::world& world);
	};
}