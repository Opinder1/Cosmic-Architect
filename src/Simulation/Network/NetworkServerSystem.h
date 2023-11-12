#pragma once

namespace sim
{
	class Simulation;

	struct SimulationTickEvent;
	struct SimulationRequestStopMessage;
	struct SimulationStopEvent;
	struct StartNetworkServerMessage;
	struct StopNetworkServerMessage;

	struct NetworkServerSystem
	{
		static void OnInitialize(Simulation& simulation);

		static void OnShutdown(Simulation& simulation);

		static void OnSimulationTick(Simulation& simulation, const SimulationTickEvent& event);

		static void OnSimulationRequestStop(Simulation& simulation, const SimulationRequestStopMessage& event);

		static void OnSimulationStop(Simulation& simulation, const SimulationStopEvent& event);

		static void OnStartNetworkServer(Simulation& simulation, const StartNetworkServerMessage& event);

		static void OnStopNetworkServer(Simulation& simulation, const StopNetworkServerMessage& event);
	};
}