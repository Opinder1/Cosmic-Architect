#pragma once

namespace flecs
{
	struct world;
}

namespace voxel_game
{
	struct RenderModule
	{
		RenderModule(flecs::world& world);
	};
}