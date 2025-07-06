#include "SimulationModule.h"
#include "Config.h"

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
		data.entity->*&entity::CRelationship::parent = entity::Ref();
	}

	void Initialize(Simulation& simulation)
	{
		simulation.processor_count = godot::OS::get_singleton()->get_processor_count();
		simulation.worker_count = simulation.processor_count;

		simulation.entity_factory.AddCallback<>(entity::Event::BeginLoad, cb::Bind<OnLoadEntity>());
		simulation.entity_factory.AddCallback<>(entity::Event::BeginUnload, cb::Bind<OnUnloadEntity>());
		simulation.entity_factory.AddCallback<entity::CRelationship>(entity::Event::BeginUnload, cb::Bind<OnUnloadChildEntity>());
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

		if (simulation.frame_start_time - simulation.last_config_save > 10s)
		{
			simulation::SaveJsonConfig(simulation.config);
			simulation.last_config_save = simulation.frame_start_time;
		}
	}

	void WorkerUpdate(Simulation& simulation, size_t index)
	{

	}

	void SetPath(Simulation& simulation, const godot::String& path)
	{
		simulation.path = path;

		simulation::InitializeConfig(simulation.config, simulation.path.path_join("config.json"), GetConfigDefaults());
		simulation.last_config_save = simulation.frame_start_time;
	}
}