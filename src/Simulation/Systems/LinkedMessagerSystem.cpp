#include "LinkedMessagerSystem.h"

#include "Simulation/Simulation.h"
#include "Simulation/Components.h"
#include "Simulation/Events.h"

namespace sim
{
	LinkedMessagerSystem::LinkedMessagerSystem(Simulation& simulation) :
		System(simulation)
	{
		Subscribe<&LinkedMessagerSystem::OnTick>();
	}

	LinkedMessagerSystem::~LinkedMessagerSystem()
	{

	}

	void LinkedMessagerSystem::OnTick(const TickEvent& event)
	{

	}

	void LinkedMessagerSystem::SendMessage(Simulation& simulation, LinkedMessagerComponent& linked_messager, const MessagePtr& message)
	{
		linked_messager.queued_messages.push_back(message);
	}

	void LinkedMessagerSystem::SendMessage(Simulation& simulation, LinkedMessagerComponent& linked_messager, const MessageQueue& messages)
	{
		linked_messager.queued_messages.insert(linked_messager.queued_messages.end(), messages.begin(), messages.end());
	}
}