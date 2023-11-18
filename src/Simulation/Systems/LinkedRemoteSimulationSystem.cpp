#include "LinkedRemoteSimulationSystem.h"

#include "Simulation/Simulation.h"
#include "Simulation/Components.h"
#include "Simulation/Events.h"
#include "Simulation/Globals.h"

#include "Simulation/Message/Message.h"

#include "Simulation/Network/Events.h"

namespace sim
{
	void LinkedRemoteSimulationSystem::OnInitialize(Simulation& simulation)
	{
		simulation.Subscribe(cb::BindParam<&LinkedRemoteSimulationSystem::OnSimulationTick>(simulation));
	}

	void LinkedRemoteSimulationSystem::OnShutdown(Simulation& simulation)
	{
		simulation.Unsubscribe(cb::BindParam<&LinkedRemoteSimulationSystem::OnSimulationTick>(simulation));
	}

	void LinkedRemoteSimulationSystem::OnSimulationTick(Simulation& simulation, const SimulationTickEvent& event)
	{
		auto& network_simulation = simulation.Global<NetworkSimulationGlobal>();

		for (auto&& [entity, linked_messager] : simulation.Registry().view<LinkedMessagerComponent, LinkedThreadSimulationComponent>().each())
		{
			if (!linked_messager.messager_id.IsEmpty())
			{
				auto message = std::make_shared<MessageRemoteSimulationMessage>(simulation, linked_messager.messager_id);

				// This clears queued_messages and is only swapping pointers which is fast
				message->messages.swap(linked_messager.queued_messages);

				simulation.PostMessageToOther(network_simulation.id, message);
			}
		}
	}
}