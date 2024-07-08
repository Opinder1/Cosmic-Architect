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

	struct EntityCreateCommand
	{
		flecs::entity_t prefab;
		flecs::entity_t parent;
	};

	struct EntityDestroyCommand
	{
		flecs::entity_t entity;
	};

	struct EntityThreadCommands : Nocopy
	{
		struct ThreadData
		{
			std::vector<EntityCreateCommand> create_commands;
			std::vector<EntityDestroyCommand> destroy_commands;
		};

		PerThread<ThreadData> threads;
	};
}