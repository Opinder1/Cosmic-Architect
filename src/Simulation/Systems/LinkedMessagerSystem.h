#pragma once

#include "Simulation/System.h"

namespace sim
{
	struct Message;
	struct TickEvent;

	using MessagePtr = std::shared_ptr<Message>;
	using MessageQueue = std::vector<MessagePtr>;

	class LinkedMessagerSystem : public System
	{
	public:
		LinkedMessagerSystem(Simulation& simulation);
		~LinkedMessagerSystem();

	private:
		void OnTick(const TickEvent& event);
	};

	void SendMessage(Simulation& simulation, LinkedMessagerComponent& linked_messager, const MessagePtr& message);

	void SendMessage(Simulation& simulation, LinkedMessagerComponent& linked_messager, const MessageQueue& messages);
}