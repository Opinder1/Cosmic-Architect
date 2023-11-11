#pragma once

#include "Simulation/System.h"
#include "Simulation/UUID.h"

namespace sim
{
	struct PostTickEvent;

	class LinkedRemoteSimulationSystem : public System
	{
	public:
		LinkedRemoteSimulationSystem(Simulation& simulation, UUID network_simulation);
		~LinkedRemoteSimulationSystem();

	private:
		void OnPostTick(const PostTickEvent& event);

	private:
		UUID m_network_simulation;
	};
}