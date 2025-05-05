#include "LoadingModule.h"
#include "LoadingWorld.h"
#include "LoadingComponents.h"

#include "Entity/EntityPoly.h"

#include "UniverseSimulation.h"

#include "Util/Debug.h"

namespace voxel_game::loading
{
	void Initialize(Simulation& simulation)
	{

	}

	void Uninitialize(Simulation& simulation)
	{
		for (entity::WRef entity : simulation.entities)
		{
			if (entity.Has<CStreamable>())
			{
				DEBUG_ASSERT(entity->*&loading::CStreamable::state == loading::State::Unloaded, "Entity should have been unloaded");
			}
		};
	}

	void Update(Simulation& simulation)
	{
		for (entity::WRef entity : simulation.entities)
		{
			if (entity.Has<CStreamable>())
			{
				if (entity->*&loading::CStreamable::state == loading::State::Unloaded)
				{
					simulation.entity_factory.DoEvent(simulation, entity, entity::Event::Destroy);

					unordered_erase(simulation.entities, entity);
				}
			}
		}
	}

	void WorkerUpdate(Simulation& simulation, size_t index)
	{

	}

	void WorldUpdate(Simulation& simulation, spatial3d::WorldPtr world)
	{

	}

	void ScaleUpdate(Simulation& simulation, spatial3d::ScalePtr scale)
	{
		ScaleDoLoadCommands(simulation, scale);
	}
}