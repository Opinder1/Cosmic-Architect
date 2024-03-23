#pragma once

namespace flecs
{
	struct world;
}

namespace voxel_game
{
	struct GodotRenderModule
	{
		GodotRenderModule(flecs::world& world);
	};
}