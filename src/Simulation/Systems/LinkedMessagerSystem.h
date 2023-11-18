#pragma once

#include <memory>
#include <vector>

namespace sim
{
	class Simulation;

	struct MessagePtr;
	struct MessageQueue;
	struct SimulationTickEvent;
	struct LinkedMessagerComponent;

	struct LinkedMessagerSystem
	{
		static void OnInitialize(Simulation& simulation);

		static void OnShutdown(Simulation& simulation);

		static void OnSimulationTick(Simulation& simulation, const SimulationTickEvent& event);

		static void SendMessage(LinkedMessagerComponent& linked_messager, const MessagePtr& message);

		static void SendMessages(LinkedMessagerComponent& linked_messager, const MessageQueue& messages);
	};
}