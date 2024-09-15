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
	}

	ThreadEntityPool& GetThreadEntityPool(flecs::world_t* stage)
	{
		DEBUG_ASSERT(flecs_poly_is(stage, ecs_stage_t), "We should be only using this function when in multithreaded systems");

		flecs::world stage_obj(stage);

		ThreadEntityPools* entity_pools = stage_obj.get_mut<ThreadEntityPools>();

		DEBUG_ASSERT(entity_pools != nullptr, "The thread entity pools should have been initialized before we call this");

		size_t thread_id = stage_obj.get_stage_id();

		DEBUG_ASSERT(thread_id < entity_pools->threads.size(), "We should be calling this for a worker thread in the worker thread range");

		return entity_pools->threads[thread_id];
	}

	flecs::entity_t CreateThreadEntity(ThreadEntityPool& entity_pool)
	{
		DEBUG_ASSERT(!entity_pool.new_entities.is_empty(), "Ran out of entities to spawn on this thread. We should increase the entity pool size");

		flecs::entity_t entity = entity_pool.new_entities.back();
		entity_pool.new_entities.pop_back();

		return entity;
	}

	flecs::entity_t CreateThreadEntity(flecs::world_t* stage)
	{
		if (flecs_poly_is(stage, ecs_world_t))
		{
			return ecs_new(stage);
		}

		return CreateThreadEntity(GetThreadEntityPool(stage));
	}
}