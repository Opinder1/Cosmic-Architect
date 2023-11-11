#include "LinkedSimulationSystem.h"

#include "Simulation/Simulation.h"
#include "Simulation/Components.h"
#include "Simulation/Events.h"

namespace sim
{
	LinkedSimulationSystem::LinkedSimulationSystem(Simulation& simulation) :
		System(simulation)
	{
		Sim().Subscribe(cb::Bind<&LinkedSimulationSystem::OnPostTick>(this));
	}

	LinkedSimulationSystem::~LinkedSimulationSystem()
	{
		Sim().Unsubscribe(cb::Bind<&LinkedSimulationSystem::OnPostTick>(this));
	}

	void LinkedSimulationSystem::OnPostTick(const PostTickEvent& event)
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