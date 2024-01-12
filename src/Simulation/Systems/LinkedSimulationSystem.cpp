#include "LinkedSimulationSystem.h"

#include "Simulation/Simulation.h"
#include "Simulation/Components.h"
#include "Simulation/Events.h"

namespace sim
{
	void LinkedSimulationSystem::OnInitialize(Simulation& simulation)
	{
		simulation.messager.Subscribe(cb::BindParam<&LinkedSimulationSystem::OnSimulationTick>(simulation));
	}

	void LinkedSimulationSystem::OnShutdown(Simulation& simulation)
	{
		simulation.messager.Unsubscribe(cb::BindParam<&LinkedSimulationSystem::OnSimulationTick>(simulation));
	}

	void LinkedSimulationSystem::OnSimulationTick(Simulation& simulation, const SimulationTickEvent& event)
	{
		for (auto&& [entity, linked_messager] : simulation.registry.view<LinkedMessagerComponent, LinkedThreadSimulationComponent>().each())
		{
			if (!linked_messager.messager_id.IsEmpty())
			{
				simulation.messager.PostMessagesToOther(linked_messager.messager_id, linked_messager.queued_messages);

				linked_messager.queued_messages.clear();
			}
		}
	}
}