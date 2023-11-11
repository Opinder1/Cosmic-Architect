#include "LinkedRemoteSimulationSystem.h"

#include "Simulation/Simulation.h"
#include "Simulation/Components.h"
#include "Simulation/Events.h"

#include "Simulation/Message/Message.h"

#include "Simulation/Network/Events.h"

namespace sim
{
	LinkedRemoteSimulationSystem::LinkedRemoteSimulationSystem(Simulation& simulation, UUID network_simulation) :
		System(simulation),
		m_network_simulation(network_simulation)
	{
		Sim().Subscribe(cb::Bind<&LinkedRemoteSimulationSystem::OnPostTick>(this));
	}

	LinkedRemoteSimulationSystem::~LinkedRemoteSimulationSystem()
	{
		Sim().Unsubscribe(cb::Bind<&LinkedRemoteSimulationSystem::OnPostTick>(this));
	}

	void LinkedRemoteSimulationSystem::OnPostTick(const PostTickEvent& event)
	{
		for (auto&& [entity, linked_messager] : Registry().view<LinkedMessagerComponent, LinkedSimulationComponent>().each())
		{
			if (!linked_messager.id.IsEmpty())
			{
				auto message = std::make_shared<MessageRemoteSimulationMessage>(Sim(), linked_messager.id);

				// This clears queued_messages and is only swapping pointers which is fast
				message->messages.swap(linked_messager.queued_messages);

				Sim().PostMessageToOther(m_network_simulation, message);
			}
		}
	}
}