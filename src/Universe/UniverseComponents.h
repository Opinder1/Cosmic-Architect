#pragma once

namespace flecs
{
	struct world;
}

namespace voxel_game
{
	struct UniverseComponent {};

	struct UniverseObjectComponent {};

	struct UniverseComponents
	{
		UniverseComponents(flecs::world& world);
	};
}