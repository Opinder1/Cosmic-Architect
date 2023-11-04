#pragma once

#include "Simulation/System.h"

namespace sim
{
	struct TickEvent;

	class LinkedSimulationSystem : public System
	{
	public:
		LinkedSimulationSystem(Simulation& simulation);
		~LinkedSimulationSystem();

	private:
		void OnTick(const TickEvent& event);
	};
}