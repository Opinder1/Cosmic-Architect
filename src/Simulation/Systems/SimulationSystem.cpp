#include "SimulationSystem.h"

#include "Simulation/Simulation.h"
#include "Simulation/Components.h"
#include "Simulation/Events.h"

namespace sim
{
	SimulationSystem::SimulationSystem(Simulation& simulation) :
		System(simulation)
	{
		Sim().Subscribe(cb::Bind<&SimulationSystem::OnTick>(*this));
	}

	SimulationSystem::~SimulationSystem()
	{
		Sim().Unsubscribe(cb::Bind<&SimulationSystem::OnTick>(*this));
	}

	void SimulationSystem::OnTick(const TickEvent& event)
	{
		for (auto&& [entity, linked_messager] : Registry().view<LinkedMessagerComponent, LinkedSimulationComponent>().each())
		{
			if (!linked_messager.id.IsEmpty())
			{
				Sim().PostMessagesToOther(linked_messager.id, linked_messager.queued_messages);

				linked_messager.queued_messages.clear();
			}
		}
	}
}