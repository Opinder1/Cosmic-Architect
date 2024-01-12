#include "LinkedMessagerSystem.h"

#include "Simulation/Simulation.h"
#include "Simulation/Components.h"
#include "Simulation/Events.h"

namespace sim
{
	void LinkedMessagerSystem::OnInitialize(Simulation& simulation)
	{
		simulation.messager.Subscribe(cb::BindParam<&LinkedMessagerSystem::OnSimulationTick>(simulation));
	}

	void LinkedMessagerSystem::OnShutdown(Simulation& simulation)
	{
		simulation.messager.Unsubscribe(cb::BindParam<&LinkedMessagerSystem::OnSimulationTick>(simulation));
	}

	void LinkedMessagerSystem::OnSimulationTick(Simulation& simulation, const SimulationTickEvent& event)
	{

	}

	void LinkedMessagerSystem::SendMessage(LinkedMessagerComponent& linked_messager, const MessagePtr& message)
	{
		linked_messager.queued_messages.push_back(message);
	}

	void LinkedMessagerSystem::SendMessages(LinkedMessagerComponent& linked_messager, const MessageQueue& messages)
	{
		linked_messager.queued_messages.insert(linked_messager.queued_messages.end(), messages.begin(), messages.end());
	}
}