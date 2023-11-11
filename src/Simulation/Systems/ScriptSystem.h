#pragma once

#include "Simulation/System.h"

namespace sim
{
	struct SimulationTickEvent;

	class ScriptSystem : public System
	{
	public:
		ScriptSystem(Simulation& simulation);
		~ScriptSystem();
	};
}