#pragma once

namespace sim
{
	class ByteStream;
	class Simulation;

	struct SimulationTickEvent;

	struct NetworkPeerSystem
	{
		static void OnInitialize(Simulation& simulation);

		static void OnShutdown(Simulation& simulation);

		static void OnSimulationTick(Simulation& simulation, const SimulationTickEvent& event);

		static bool LoadMessage(Simulation& simulation, ByteStream& stream);
	};
}