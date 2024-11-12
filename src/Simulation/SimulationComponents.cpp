#include "SimulationComponents.h"

#include <flecs/flecs.h>

namespace voxel_game::sim
{
	Components::Components(flecs::world& world)
	{
		world.module<Components>();
		
		world.component<GlobalTime>();
		world.component<LocalTime>();
		world.component<ThreadEntityPools>();

		world.component<ThreadEntityPools>()
			.on_remove([world = world.c_ptr()](ThreadEntityPools& pools)
		{
			for (ThreadEntityPool& pool : pools.threads)
			{
				for (flecs::entity_t entity : pool.new_entities)
				{
					flecs::entity(world, entity).destruct();
				}

				pool.new_entities.clear();
			}
		});
	}

	ThreadEntityPool& GetThreadEntityPool(ThreadEntityPools& pools, flecs::world_t* stage)
	{
		DEBUG_ASSERT(flecs_poly_is(stage, ecs_stage_t), "We should be only using this function when in multithreaded systems");

		size_t thread_id = ecs_stage_get_id(stage);

		DEBUG_ASSERT(thread_id < pools.threads.size(), "We should be calling this for a worker thread in the worker thread range");

		return pools.threads[thread_id];
	}

	flecs::entity_t CreateThreadEntity(ThreadEntityPool& entity_pool)
	{
		DEBUG_ASSERT(!entity_pool.new_entities.is_empty(), "Ran out of entities to spawn on this thread. We should increase the entity pool size");

		flecs::entity_t entity = entity_pool.new_entities.back();
		entity_pool.new_entities.pop_back();

		return entity;
	}
}