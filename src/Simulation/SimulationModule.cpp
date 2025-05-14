#include "SimulationModule.h"

#include "Entity/EntityComponents.h"

#include "UniverseSimulation.h"

#include <godot_cpp/classes/os.hpp>

namespace voxel_game::simulation
{
	void OnLoadEntity(Simulation& simulation, entity::EventData& data)
	{
		simulation.updating_entities.push_back(entity::Ref(data.entity));
	}

	void OnUnloadEntity(Simulation& simulation, entity::EventData& data)
	{
		unordered_erase(simulation.updating_entities, data.entity);
	}

	void OnUnloadChildEntity(Simulation& simulation, entity::EventData& data)
	{
		data.entity->*&entity::CParent::parent = entity::Ref();
	}

	void Initialize(Simulation& simulation)
	{
		simulation.processor_count = godot::OS::get_singleton()->get_processor_count();
		simulation.worker_count = simulation.processor_count;

		simulation.entity_factory.AddCallback<>(entity::Event::BeginLoad, cb::Bind<OnLoadEntity>());
		simulation.entity_factory.AddCallback<>(entity::Event::BeginUnload, cb::Bind<OnUnloadEntity>());
		simulation.entity_factory.AddCallback<entity::CParent>(entity::Event::BeginUnload, cb::Bind<OnUnloadChildEntity>());
	}

	void Uninitialize(Simulation& simulation)
	{
	}

	bool IsUnloadDone(Simulation& simulation)
	{
		return simulation.updating_entities.empty();
	}

	void Update(Simulation& simulation)
	{
		simulation.frame_index++;
		simulation.frame_start_time = Clock::now();
	}

	void WorkerUpdate(Simulation& simulation, size_t index)
	{

	}
}