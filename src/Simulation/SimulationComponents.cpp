#include "SimulationComponents.h"

#include <flecs/flecs.h>

namespace voxel_game
{
	SimulationComponents::SimulationComponents(flecs::world& world)
	{
		world.module<SimulationComponents>();
		
		world.component<SimulationTime>();
		world.component<LocalTime>();
		world.component<ThreadEntityPools>();
	}

	ThreadEntityPool* GetThreadEntityPool(flecs::world_t* stage)
	{
		flecs::world stage_obj(stage);

		ThreadEntityPools* entity_pools = stage_obj.get_mut<ThreadEntityPools>();

		if (entity_pools == nullptr)
		{
			return nullptr;
		}

		size_t thread_id = stage_obj.get_stage_id();

		if (thread_id >= entity_pools->threads.size())
		{
			return nullptr;
		}

		return &entity_pools->threads[thread_id];
	}
}