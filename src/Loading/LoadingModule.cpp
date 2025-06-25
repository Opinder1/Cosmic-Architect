#include "LoadingModule.h"
#include "LoadingWorld.h"
#include "LoadingComponents.h"

#include "Entity/EntityPoly.h"
#include "Entity/EntityComponents.h"
#include "Entity/EntityModule.h"

#include "UniverseSimulation.h"

#include "Util/Debug.h"

namespace voxel_game::loading
{
	void OnLoadStreamableEntity(Simulation& simulation, entity::EventData& data)
	{
		data.entity->*&CStreamable::state = State::Loading;
	}

	void OnUnloadStreamableEntity(Simulation& simulation, entity::EventData& data)
	{
		data.entity->*&CStreamable::state = State::Unloading;
		simulation.unloading_entities.push_back(entity::Ref(data.entity));
	}

	void OnUpdateStreamableEntity(Simulation& simulation, entity::EventData& data)
	{
		switch (data.entity->*&CStreamable::state)
		{
		case State::Loading:
			if (data.entity->*&loading::CStreamable::tasks == 0)
			{
				data.entity->*&CStreamable::state = State::Loaded;
			}
			break;

		case State::Unloading:
			if (data.entity->*&loading::CStreamable::tasks == 0)
			{
				data.entity->*&CStreamable::state = State::Unloaded;
			}
			break;
		}
	}

	void Initialize(Simulation& simulation)
	{
		simulation.entity_factory.AddCallback<CStreamable>(entity::Event::BeginLoad, cb::Bind<OnLoadStreamableEntity>());
		simulation.entity_factory.AddCallback<CStreamable>(entity::Event::BeginUnload, cb::Bind<OnUnloadStreamableEntity>());
		simulation.entity_factory.AddCallback<CStreamable>(entity::Event::UpdateTask, cb::Bind<OnUpdateStreamableEntity>());
		simulation.entity_factory.AddCallback<CStreamable>(entity::Event::UnloadTask, cb::Bind<OnUpdateStreamableEntity>());
	}

	void Uninitialize(Simulation& simulation)
	{
		DEBUG_ASSERT(simulation.unloading_entities.empty(), "We should have unloaded all entities in uninitialize loop");
	}

	bool IsUnloadDone(Simulation& simulation)
	{
		return true;
	}

	void Update(Simulation& simulation)
	{
		if (simulation.unloading)
		{
			// Unload all entities in the world. The entities memory still exists while there are references
			for (entity::WRef entity : simulation.loading_entities)
			{
				entity::OnUnloadEntity(simulation, entity);
			}
		}

		for (auto it = simulation.unloading_entities.begin(); it != simulation.unloading_entities.end();)
		{
			entity::WRef entity = *it;

			if (!entity.Has<CStreamable>() || entity->*&CStreamable::state == State::Unloaded)
			{
				it = simulation.unloading_entities.erase(it);
			}
			else
			{
				it++;
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