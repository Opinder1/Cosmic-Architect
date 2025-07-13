#include "SimulationModule.h"
#include "Config.h"

#include "Entity/EntityComponents.h"

#include "UniverseSimulation.h"

#include <godot_cpp/classes/os.hpp>

namespace voxel_game::simulation
{
	thread_local ThreadContext* thread_context = nullptr;

	void SetContext(ThreadContext& context)
	{
		thread_context = &context;
	}

	ThreadContext& GetContext()
	{
		DEBUG_ASSERT(thread_context != nullptr, "This thread doesn't have a context yet");
		return *thread_context;
	}

	void OnLoadEntity(Simulation& simulation, entity::WRef entity)
	{
		simulation.updating_entities.push_back(entity::Ref(entity));
	}

	void OnUnloadEntity(Simulation& simulation, entity::WRef entity)
	{
		unordered_erase(simulation.updating_entities, entity);
	}

	void OnUnloadChildEntity(Simulation& simulation, entity::WRef entity)
	{
		entity->*&entity::CRelationship::parent = entity::Ref();
	}

	void Initialize(Simulation& simulation)
	{
		simulation.processor_count = godot::OS::get_singleton()->get_processor_count();
		simulation.worker_count = simulation.processor_count;

		simulation.thread_contexts.resize(simulation.worker_count);

		SetContext(simulation.thread_contexts[0]);

		simulation.entity_factory.AddCallback<>(PolyEvent::BeginLoad, cb::BindArg<OnLoadEntity>(simulation));
		simulation.entity_factory.AddCallback<>(PolyEvent::BeginUnload, cb::BindArg<OnUnloadEntity>(simulation));
		simulation.entity_factory.AddCallback<entity::CRelationship>(PolyEvent::BeginUnload, cb::BindArg<OnUnloadChildEntity>(simulation));
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