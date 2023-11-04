#pragma once

#include "Simulation/System.h"

namespace sim
{
	struct SimulationTickEvent;

	class TickSystem : public System
	{
	public:
		TickSystem(Simulation& simulation);
		~TickSystem();

	private:
		void OnSimulationTick(const SimulationTickEvent& event);
	};
}