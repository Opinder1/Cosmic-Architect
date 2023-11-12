#pragma once

namespace sim
{
	class Simulation;

	struct SimulationTickEvent;

	class ScriptSystem
	{
	public:
		static void OnInitialize(Simulation& simulation);
		static void OnShutdown(Simulation& simulation);
	};
}