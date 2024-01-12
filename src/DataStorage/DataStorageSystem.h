#pragma once

namespace sim
{
	class Simulation;
	class UUID;
}

struct DataStorageSystem
{
	static void OnInitialize(sim::Simulation& simulation);

	static void OnShutdown(sim::Simulation& simulation);
};