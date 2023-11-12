#include "LinkedSimulationSystem.h"

#include "Simulation/Simulation.h"
#include "Simulation/Components.h"
#include "Simulation/Events.h"

namespace sim
{
	void LinkedSimulationSystem::OnInitialize(Simulation& simulation)
	{
		simulation.Subscribe(cb::BindParam<&LinkedSimulationSystem::OnSimulationTick>(simulation));
	}

	void LinkedSimulationSystem::OnShutdown(Simulation& simulation)
	{
		simulation.Unsubscribe(cb::BindParam<&LinkedSimulationSystem::OnSimulationTick>(simulation));
	}

	void LinkedSimulationSystem::OnSimulationTick(Simulation& simulation, const SimulationTickEvent& event)
	{
		for (auto&& [entity, linked_messager] : simulation.Registry().view<LinkedMessagerComponent, LinkedSimulationComponent>().each())
		{
			if (!linked_messager.id.IsEmpty())
			{
				simulation.PostMessagesToOther(linked_messager.id, linked_messager.queued_messages);

				linked_messager.queued_messages.clear();
			}
		}
	}
}