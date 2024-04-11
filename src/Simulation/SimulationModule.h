#pragma once

namespace flecs
{
	struct iter;
	struct world;
}

namespace voxel_game
{
	struct SimulationModule
	{
		SimulationModule(flecs::world& world);
	};
}