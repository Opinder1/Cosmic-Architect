#pragma once

namespace sim
{
	class Simulation;

	struct SimulationTickEvent;

	struct NewEntitiesSystem
	{
		static void OnInitialize(Simulation& simulation);

		static void OnShutdown(Simulation& simulation);

		static void OnSimulationTick(Simulation& simulation, const SimulationTickEvent& event);
	};
}