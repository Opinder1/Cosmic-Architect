#pragma once

#include "Simulation/System.h"

#include <robin_hood/robin_hood.h>

namespace sim
{
	struct TickEvent;
	struct SimulationRequestStopMessage;
	struct SimulationStopEvent;
	struct StartNetworkServerMessage;
	struct StopNetworkServerMessage;

	class NetworkServerSystem : public System
	{
	public:
		NetworkServerSystem(Simulation& simulation);
		~NetworkServerSystem();

	private:
		void OnTick(const TickEvent& event);

		void OnSimulationRequestStop(const SimulationRequestStopMessage& event);

		void OnSimulationStop(const SimulationStopEvent& event);

		void OnStartNetworkServer(const StartNetworkServerMessage& event);

		void OnStopNetworkServer(const StopNetworkServerMessage& event);

	private:
		robin_hood::unordered_flat_map<uint16_t, entt::entity> m_servers; // Map for ports to servers
	};
}