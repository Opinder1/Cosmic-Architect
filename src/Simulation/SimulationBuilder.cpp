#include "SimulationBuilder.h"
#include "Simulation.h"

#include "Systems/NewEntitiesSystem.h"
#include "Systems/DeletedEntitiesSystem.h"
#include "Systems/LinkedMessagerSystem.h"
#include "Systems/LinkedSimulationSystem.h"
#include "Systems/LinkedRemoteSimulationSystem.h"

namespace sim
{
	SimulationBuilder::SimulationBuilder()
	{

	}

	void SimulationBuilder::AddDefaultSystems(Simulation& simulation)
	{
		simulation.messager.SetTargetTicksPerSecond(60);

		AddSystem<NewEntitiesSystem>(simulation);
		AddSystem<DeletedEntitiesSystem>(simulation);
		AddSystem<LinkedMessagerSystem>(simulation);
		AddSystem<LinkedSimulationSystem>(simulation);
		AddSystem<LinkedRemoteSimulationSystem>(simulation);
	}

	void SimulationBuilder::AddSystem(Simulation& simulation, const SimulationApplicator& initialize, const SimulationApplicator& shutdown)
	{
		initialize(simulation);

		simulation.system_shutdowns.push_back(shutdown);
	}

	DirectorySimulationBuilder::DirectorySimulationBuilder(const godot::String& path) :
		m_path(path)
	{

	}

	void DirectorySimulationBuilder::Build(Simulation& simulation) const
	{
		godot::Ref<godot::DirAccess> directory = godot::DirAccess::open(m_path);

		if (directory.is_valid())
		{
			Build(simulation, **directory);
		}
	}
}