#pragma once

#include "Util/PerThread.h"
#include "Util/Time.h"
#include "Util/Nocopy.h"

#include <flecs/flecs.h>

#include <vector>

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

	struct SimulationTime
	{
		uint64_t frame_index = 0;
		Clock::time_point frame_start;
	};

	struct LocalTime
	{
		Clock::time_point epoch;
	};

	struct EntityLoadCommand
	{
		flecs::entity_t prefab;
		const char* name;
	};

	struct LoaderThreadData
	{
		std::vector<EntityLoadCommand> load_commands;
		std::vector<flecs::entity_t> unload_commands;
	};

	struct EntityLoaderContext : Nocopy
	{
		PerThread<LoaderThreadData> threads;
	};
}