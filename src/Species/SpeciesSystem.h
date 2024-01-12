#pragma once

namespace sim
{
	class Simulation;
	class UUID;
}
struct RenderEvent;

struct SpeciesSystem
{
	static void OnInitialize(sim::Simulation& simulation);

	static void OnShutdown(sim::Simulation& simulation);

	static void GenerateSpecies(sim::Simulation& simulation, sim::UUID id);
};