#pragma once

#include "Simulation/System.h"

namespace sim
{
	struct PostTickEvent;

	class LinkedSimulationSystem : public System
	{
	public:
		LinkedSimulationSystem(Simulation& simulation);
		~LinkedSimulationSystem();

	private:
		void OnPostTick(const PostTickEvent& event);
	};
}