#pragma once

namespace flecs
{
	struct world;
}

namespace voxel_game
{
	struct SimulationModule
	{
		SimulationModule(flecs::world& world);
	};
}