#pragma once

namespace flecs
{
	struct world;
}

namespace voxel_game::galaxy
{
	struct Module
	{
		Module(flecs::world& world);
	};
}