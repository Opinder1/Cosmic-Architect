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

	void ThreadEntityPool::SetWorld(flecs::world_t* world)
	{
		m_world = world;
	}

	void ThreadEntityPool::AllocateEntities()
	{
		size_t new_required = m_new_entities.capacity() - m_new_entities.size();

		for (size_t i = 0; i < new_required; i++)
		{
			m_new_entities.push_back(ecs_new(m_world));
		}
	}

	void ThreadEntityPool::ClearEntities()
	{
		for (flecs::entity_t entity : m_new_entities)
		{
			ecs_delete(m_world, entity);
		}

		m_new_entities.clear();
	}

	flecs::entity ThreadEntityPool::CreateEntity()
	{
		DEBUG_ASSERT(!m_new_entities.is_empty(), "Ran out of entities to spawn on this thread. We should increase the entity pool size");

		flecs::entity_t entity = m_new_entities.back();
		m_new_entities.pop_back();

		return flecs::entity(m_world, entity);
	}
}