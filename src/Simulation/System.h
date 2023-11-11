#pragma once

#include "Util/Nocopy.h"

#include "Simulation.h"

namespace sim
{
	class Simulation;
	class UUID;

	struct SimulationRequestStopMessage;
	struct SimulationStopEvent;

	class System : nocopy
	{
	public:
		System(Simulation& simulation);
		virtual ~System() = 0;

		// Simulation accessor with easy to type name
		Simulation& Sim();

		// Registry accessor with easy to type name
		entt::registry& Registry();

		// Global accessor with easy to type name
		template<class T>
		T& Global() { return Sim().Global<T>(); }

		// UUID generate function for simulation
		UUID GenerateUUID();

	private:
		Simulation& m_simulation;

		entt::registry& m_registry;
	};
}