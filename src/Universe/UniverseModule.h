#pragma once

namespace flecs
{
	struct world;
}

namespace voxel_game
{
	struct UniverseModule
	{
		UniverseModule(flecs::world& world);

		static void AddComponents(flecs::world& world);

		static void AddRelationships(flecs::world& world);
	};
}