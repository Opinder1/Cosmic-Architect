#include "LinkedMessagerSystem.h"

#include "Simulation/Simulation.h"
#include "Simulation/Components.h"
#include "Simulation/Events.h"

namespace sim
{
	LinkedMessagerSystem::LinkedMessagerSystem(Simulation& simulation) :
		System(simulation)
	{
		Sim().Subscribe(cb::Bind<&LinkedMessagerSystem::OnTick>(*this));
	}

	LinkedMessagerSystem::~LinkedMessagerSystem()
	{
		Sim().Unsubscribe(cb::Bind<&LinkedMessagerSystem::OnTick>(*this));
	}

	void LinkedMessagerSystem::OnTick(const TickEvent& event)
	{

	}

	void SendMessage(Simulation& simulation, LinkedMessagerComponent& linked_messager, const MessagePtr& message)
	{
		linked_messager.queued_messages.push_back(message);
	}

	void SendMessage(Simulation& simulation, LinkedMessagerComponent& linked_messager, const MessageQueue& messages)
	{
		linked_messager.queued_messages.insert(linked_messager.queued_messages.end(), messages.begin(), messages.end());
	}
}