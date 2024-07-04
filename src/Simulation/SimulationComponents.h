#pragma once

#include "Util/Time.h"
#include "Util/Nocopy.h"

#include <flecs/flecs.h>

#include <vector>
#include <array>

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

	struct EntityLoadCommands
	{
		std::vector<EntityLoadCommand> load_commands;
	};

	struct EntityLoaderContext : Nocopy
	{
		std::array<EntityLoadCommands, 16> thread_buffers;
	};
}