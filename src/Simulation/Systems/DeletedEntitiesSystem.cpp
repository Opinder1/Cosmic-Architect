#include "DeletedEntitiesSystem.h"

#include "Simulation/Simulation.h"
#include "Simulation/Components.h"
#include "Simulation/Events.h"

namespace sim
{
	void DeletedEntitiesSystem::OnInitialize(Simulation& simulation)
	{
		simulation.dispatcher.Subscribe(cb::BindParam<&DeletedEntitiesSystem::OnSimulationTick>(simulation));
	}

	void DeletedEntitiesSystem::OnShutdown(Simulation& simulation)
	{
		simulation.dispatcher.Unsubscribe(cb::BindParam<&DeletedEntitiesSystem::OnSimulationTick>(simulation));
	}

	void DeletedEntitiesSystem::OnSimulationTick(Simulation& simulation, const SimulationTickEvent& event)
	{
		// Process new entities then remove the new entity component flag
		simulation.dispatcher.PostEvent(ProcessNewEntitiesEvent());

		simulation.registry.clear<NewComponent>();

		// Process deleted entities then remove them and their components from the registry
		simulation.dispatcher.PostEvent(ProcessDeletedEntitiesEvent());

		for (auto [entity] : simulation.registry.view<DeletedComponent>().each())
		{
			simulation.registry.destroy(entity);
		}
	}
}