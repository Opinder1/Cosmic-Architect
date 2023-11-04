#pragma once

#include "Simulation/System.h"

namespace sim
{
	struct TickEvent;

	class SimulationSystem : public System
	{
	public:
		SimulationSystem(Simulation& simulation);
		~SimulationSystem();

	private:
		void OnTick(const TickEvent& event);
	};
}