#pragma once

namespace flecs
{
	class world;
}

namespace voxel_game::loading
{
	struct Module
	{
		Module(flecs::world& world);
	};
}