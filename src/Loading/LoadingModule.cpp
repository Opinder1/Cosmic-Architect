#include "LoadingModule.h"
#include "LoadingWorld.h"
#include "LoadingComponents.h"

#include "Entity/EntityPoly.h"
#include "Entity/EntityComponents.h"

#include "UniverseSimulation.h"

#include "Util/Debug.h"

namespace voxel_game::loading
{
	void OnUnloadEntity(Simulation& simulation, entity::EventData& data)
	{
		simulation.unloading_entities.push_back(entity::Ref(data.entity));
	}

	void OnLoadStreamableEntity(Simulation& simulation, entity::EventData& data)
	{
		data.entity->*&CStreamable::state = State::Loading;
	}

	void OnUnloadStreamableEntity(Simulation& simulation, entity::EventData& data)
	{
		data.entity->*&CStreamable::state = State::Unloading;
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
		simulation.entity_factory.AddCallback(entity::Event::Unload, cb::Bind<OnUnloadEntity>());

		simulation.entity_factory.AddCallback<CStreamable>(entity::Event::Load, cb::Bind<OnLoadStreamableEntity>());
		simulation.entity_factory.AddCallback<CStreamable>(entity::Event::Unload, cb::Bind<OnUnloadStreamableEntity>());
		simulation.entity_factory.AddCallback<CStreamable>(entity::Event::Update, cb::Bind<OnUpdateStreamableEntity>());
	}

	void Uninitialize(Simulation& simulation)
	{
		DEBUG_ASSERT(simulation.unloading_entities.empty(), "We should have unloaded all entities in uninitialize loop");
	}

	void Update(Simulation& simulation)
	{
		for (auto it = simulation.unloading_entities.begin(); it != simulation.unloading_entities.end();)
		{
			entity::WRef entity = *it;

			if (!entity.Has<CStreamable>() || entity->*&CStreamable::state == State::Unloaded)
			{
				it = simulation.unloading_entities.erase(it);
				unordered_erase(simulation.entities, entity);
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