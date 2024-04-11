#pragma once

#include "Util/Time.h"

namespace flecs
{
	struct world;
}

namespace voxel_game
{
	struct SimulationComponents
	{
		SimulationComponents(flecs::world& world);
	};

	struct SimulationGlobal
	{
		uint64_t frame_index = 0;
		Clock::time_point frame_start;
	};
}