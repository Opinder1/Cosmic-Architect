#pragma once

namespace sim
{
	class Simulation;

	struct SimulationTickEvent;
}

struct ScriptSystem
{
	static void OnInitialize(sim::Simulation& simulation);

	static void OnShutdown(sim::Simulation& simulation);
};