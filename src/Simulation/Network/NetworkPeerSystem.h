#pragma once

namespace sim
{
	class ByteStream;
	class Simulation;

	struct SimulationTickEvent;

	class NetworkPeerSystem
	{
	public:
		static void OnInitialize(Simulation& simulation);

		static void OnShutdown(Simulation& simulation);

		static void OnSimulationTick(Simulation& simulation, const SimulationTickEvent& event);

		static bool LoadMessage(Simulation& simulation, ByteStream& stream);
	};
}