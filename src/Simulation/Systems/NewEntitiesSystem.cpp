#include "NewEntitiesSystem.h"

#include "Simulation/Simulation.h"
#include "Simulation/Components.h"
#include "Simulation/Events.h"

namespace sim
{
	void NewEntitiesSystem::OnInitialize(Simulation& simulation)
	{
		simulation.dispatcher.Subscribe(cb::BindParam<&NewEntitiesSystem::OnSimulationTick>(simulation));
	}

	void NewEntitiesSystem::OnShutdown(Simulation& simulation)
	{
		simulation.dispatcher.Unsubscribe(cb::BindParam<&NewEntitiesSystem::OnSimulationTick>(simulation));
	}

	void NewEntitiesSystem::OnSimulationTick(Simulation& simulation, const SimulationTickEvent& event)
	{
		// Process new entities then remove the new entity component flag
		simulation.dispatcher.PostEvent(ProcessNewEntitiesEvent());

		simulation.registry.clear<NewComponent>();
	}
}