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
		simulation.dispatcher.Subscribe(cb::BindParam<&LinkedRemoteSimulationSystem::OnSimulationTick>(simulation));
	}

	void LinkedRemoteSimulationSystem::OnShutdown(Simulation& simulation)
	{
		simulation.dispatcher.Unsubscribe(cb::BindParam<&LinkedRemoteSimulationSystem::OnSimulationTick>(simulation));
	}

	void LinkedRemoteSimulationSystem::OnSimulationTick(Simulation& simulation, const SimulationTickEvent& event)
	{
		auto* network_simulation = simulation.globals.find<NetworkSimulationGlobal>();

		if (!network_simulation)
		{
			return;
		}

		for (auto&& [entity, linked_messager] : simulation.registry.view<LinkedMessagerComponent, LinkedNetworkSimulationComponent>().each())
		{
			if (!linked_messager.messager_id.IsEmpty())
			{
				auto message = std::make_shared<MessageRemoteSimulationMessage>(simulation.messager, linked_messager.messager_id);

				// This clears queued_messages and is only swapping pointers which is fast
				message->messages.swap(linked_messager.queued_messages);

				simulation.messager.PostMessageToOther(network_simulation->id, message);
			}
		}
	}
}