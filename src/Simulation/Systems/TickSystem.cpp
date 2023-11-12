#include "TickSystem.h"

#include "Simulation/Simulation.h"
#include "Simulation/Components.h"
#include "Simulation/Events.h"

namespace sim
{
	void TickSystem::OnInitialize(Simulation& simulation)
	{
		simulation.Subscribe(cb::BindParam<&TickSystem::OnSimulationTick>(simulation));
	}

	void TickSystem::OnShutdown(Simulation& simulation)
	{
		simulation.Unsubscribe(cb::BindParam<&TickSystem::OnSimulationTick>(simulation));
	}

	void TickSystem::OnSimulationTick(Simulation& simulation, const SimulationTickEvent& event)
	{
		// Process new entities then remove the new entity component flag
		simulation.PostEvent(ProcessNewEntitiesEvent());

		simulation.Registry().clear<NewEntityComponent>();

		// Process deleted entities then remove them and their components from the registry
		simulation.PostEvent(ProcessDeletedEntitiesEvent());

		for (auto [entity] : simulation.Registry().view<DeletedEntityComponent>().each())
		{
			simulation.Registry().destroy(entity);
		}
	}
}