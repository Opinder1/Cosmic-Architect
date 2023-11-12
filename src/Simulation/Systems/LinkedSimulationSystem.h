#pragma once

namespace sim
{
	class Simulation;

	struct SimulationTickEvent;

	class LinkedSimulationSystem
	{
	public:
		static void OnInitialize(Simulation& simulation);

		static void OnShutdown(Simulation& simulation);

		static void OnSimulationTick(Simulation& simulation, const SimulationTickEvent& event);
	};
}