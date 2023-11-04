#pragma once

#include "Simulation/UUID.h"

namespace sim
{
	struct NetworkPeer;
	struct NetworkServer;

	// A singleton that manages all network connections to simulations outside of this process
	class NetworkMessager
	{
	public:
		static void Initialize();
		static void Uninitialize();

		static UUID GetSimulation();

	private:
		static UUID m_simulation_id;
	};
}