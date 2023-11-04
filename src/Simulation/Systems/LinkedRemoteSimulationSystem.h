#pragma once

#include "Simulation/System.h"
#include "Simulation/UUID.h"

namespace sim
{
	struct TickEvent;

	class LinkedRemoteSimulationSystem : public System
	{
	public:
		LinkedRemoteSimulationSystem(Simulation& simulation);
		~LinkedRemoteSimulationSystem();

	private:
		void OnTick(const TickEvent& event);

	private:
		UUID m_network_simulation;
	};
}