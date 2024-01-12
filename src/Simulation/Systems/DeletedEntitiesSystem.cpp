#include "DeletedEntitiesSystem.h"

#include "Simulation/Simulation.h"
#include "Simulation/Components.h"
#include "Simulation/Events.h"

namespace sim
{
	void DeletedEntitiesSystem::OnInitialize(Simulation& simulation)
	{
		simulation.messager.Subscribe(cb::BindParam<&DeletedEntitiesSystem::OnSimulationTick>(simulation));
	}

	void DeletedEntitiesSystem::OnShutdown(Simulation& simulation)
	{
		simulation.messager.Unsubscribe(cb::BindParam<&DeletedEntitiesSystem::OnSimulationTick>(simulation));
	}

	void DeletedEntitiesSystem::OnSimulationTick(Simulation& simulation, const SimulationTickEvent& event)
	{
		// Process new entities then remove the new entity component flag
		simulation.messager.PostEvent(ProcessNewEntitiesEvent());

		simulation.registry.clear<NewEntityComponent>();

		// Process deleted entities then remove them and their components from the registry
		simulation.messager.PostEvent(ProcessDeletedEntitiesEvent());

		for (auto [entity] : simulation.registry.view<DeletedEntityComponent>().each())
		{
			simulation.registry.destroy(entity);
		}
	}
}