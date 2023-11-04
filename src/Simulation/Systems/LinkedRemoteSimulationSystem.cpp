#include "LinkedRemoteSimulationSystem.h"

#include "Simulation/Simulation.h"
#include "Simulation/Components.h"
#include "Simulation/Events.h"

#include "Simulation/Message/Message.h"

#include "Simulation/Network/NetworkMessager.h"
#include "Simulation/Network/Events.h"

namespace sim
{
	LinkedRemoteSimulationSystem::LinkedRemoteSimulationSystem(Simulation& simulation) :
		System(simulation),
		m_network_simulation(NetworkMessager::GetSimulation())
	{
		Sim().Subscribe(cb::Bind<&LinkedRemoteSimulationSystem::OnTick>(*this));
	}

	LinkedRemoteSimulationSystem::~LinkedRemoteSimulationSystem()
	{
		Sim().Unsubscribe(cb::Bind<&LinkedRemoteSimulationSystem::OnTick>(*this));
	}

	void LinkedRemoteSimulationSystem::OnTick(const TickEvent& event)
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