#pragma once

#include <memory>
#include <vector>

namespace sim
{
	class Simulation;

	struct Message;
	struct SimulationTickEvent;
	struct LinkedMessagerComponent;

	using MessagePtr = std::shared_ptr<Message>;
	using MessageQueue = std::vector<MessagePtr>;

	struct LinkedMessagerSystem
	{
		static void OnInitialize(Simulation& simulation);

		static void OnShutdown(Simulation& simulation);

		static void OnSimulationTick(Simulation& simulation, const SimulationTickEvent& event);

		static void SendMessage(Simulation& simulation, LinkedMessagerComponent& linked_messager, const MessagePtr& message);

		static void SendMessage(Simulation& simulation, LinkedMessagerComponent& linked_messager, const MessageQueue& messages);
	};
}