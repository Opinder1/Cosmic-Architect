#include "Simulation.h"

#include "Util/Debug.h"

namespace sim
{
	Simulation::Simulation(SimulationServer& server, UUID id, double ticks_per_second) :
		messager(server, id, ticks_per_second),
		globals(registry.get_allocator())
	{}

	Simulation::~Simulation()
	{
		for (SimulationApplicator& shutdown : system_shutdowns)
		{
			shutdown(*this);
		}
	}
}