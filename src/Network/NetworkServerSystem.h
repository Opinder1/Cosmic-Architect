#pragma once

#include "Simulation/System.h"

#include <godot_cpp/classes/ref.hpp>

#include <robin_hood/robin_hood.h>

namespace godot
{
	class UDPServer;
	class DTLSServer;
	class TLSOptions;
}

namespace sim
{
	struct SimulationTickEvent;
	struct StartServerMessage;
	struct StopServerMessage;

	class NetworkServerSystem : public System
	{
	public:
		NetworkServerSystem(Simulation& simulation);
		~NetworkServerSystem();

	private:
		void OnSimulationTick(const SimulationTickEvent& event);

		void OnStartServer(const StartServerMessage& event);

		void OnStopServer(const StopServerMessage& event);

	private:
		robin_hood::unordered_flat_map<uint16_t, entt::entity> m_servers; // Map for ports to servers
	};
}