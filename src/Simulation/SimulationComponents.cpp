#include "SimulationComponents.h"

#include <flecs/flecs.h>

namespace voxel_game::sim
{
	Components::Components(flecs::world& world)
	{
		world.module<Components>();
		
		world.component<CFrame>();
		world.component<CLocalTime>();
		world.component<CPath>();
		world.component<CConfig>();
		world.component<CThreadWorker>();
		world.component<CEntityPools>();
	}

	ThreadEntityPool::ThreadEntityPool()
	{}

	void ThreadEntityPool::SetStage(flecs::world_t* stage)
	{
		m_stage = stage;
	}

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

	flecs::entity ThreadEntityPool::CreateEntity()
	{
		DEBUG_ASSERT(!m_new_entities.is_empty(), "Ran out of entities to spawn on this thread. We should increase the entity pool size");

		flecs::entity_t entity = m_new_entities.back();
		m_new_entities.pop_back();

		return flecs::entity(m_stage, entity);
	}
}