#include "SimulationComponents.h"

#include <flecs/flecs.h>

namespace voxel_game::sim
{
	Components::Components(flecs::world& world)
	{
		world.module<Components>();
		
		world.component<GlobalTime>();
		world.component<LocalTime>();
		world.component<Path>();
		world.component<Config>();
		world.component<ThreadEntityPools>();

		world.component<ThreadEntityPools>()
			.on_remove([world = world.c_ptr()](ThreadEntityPools& pools)
		{
			for (ThreadEntityPool& pool : pools.threads)
			{
				pool.ClearEntities(world);
			}
		});
	}

	ThreadEntityPool::ThreadEntityPool()
	{}

	void ThreadEntityPool::AllocateEntities(flecs::world_t* world)
	{
		size_t new_required = m_new_entities.capacity() - m_new_entities.size();

		for (size_t i = 0; i < new_required; i++)
		{
			m_new_entities.push_back(ecs_new(world));
		}
	}

	void ThreadEntityPool::ClearEntities(flecs::world_t* world)
	{
		for (flecs::entity_t entity : m_new_entities)
		{
			ecs_delete(world, entity);
		}

		m_new_entities.clear();
	}

	flecs::entity_t ThreadEntityPool::CreateThreadEntity()
	{
		DEBUG_ASSERT(!m_new_entities.is_empty(), "Ran out of entities to spawn on this thread. We should increase the entity pool size");

		flecs::entity_t entity = m_new_entities.back();
		m_new_entities.pop_back();

		return entity;
	}

	ThreadEntityPool& GetThreadEntityPool(ThreadEntityPools& pools, flecs::world_t* stage)
	{
		DEBUG_ASSERT(flecs_poly_is(stage, ecs_stage_t), "We should be only using this function when in multithreaded systems");

		size_t thread_id = ecs_stage_get_id(stage);

		DEBUG_ASSERT(thread_id < pools.threads.size(), "We should be calling this for a worker thread in the worker thread range");

		return pools.threads[thread_id];
	}
}