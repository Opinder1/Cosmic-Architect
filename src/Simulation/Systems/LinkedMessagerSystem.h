#pragma once

#include "Simulation/System.h"

namespace sim
{
	struct Message;
	struct PostTickEvent;
	struct LinkedMessagerComponent;

	using MessagePtr = std::shared_ptr<Message>;
	using MessageQueue = std::vector<MessagePtr>;

	class LinkedMessagerSystem : public System
	{
	public:
		LinkedMessagerSystem(Simulation& simulation);
		~LinkedMessagerSystem();

		static void SendMessage(Simulation& simulation, LinkedMessagerComponent& linked_messager, const MessagePtr& message);

		static void SendMessage(Simulation& simulation, LinkedMessagerComponent& linked_messager, const MessageQueue& messages);

	private:
		void OnPostTick(const PostTickEvent& event);
	};
}