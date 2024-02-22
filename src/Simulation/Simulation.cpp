#include "Simulation.h"

#include "Util/Debug.h"

namespace sim
{
	Simulation::Simulation(SimulationServer& server, UUID id) :
		server(server),
		messager(server, id, dispatcher),
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