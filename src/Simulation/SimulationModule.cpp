#include "SimulationModule.h"

#include "UniverseSimulation.h"

#include <godot_cpp/classes/os.hpp>

namespace voxel_game::simulation
{
	void Initialize(Simulation& simulation)
	{
		simulation.processor_count = godot::OS::get_singleton()->get_processor_count();
		simulation.worker_count = simulation.processor_count;
	}

	void Uninitialize(Simulation& simulation)
	{

	}

	void Update(Simulation& simulation)
	{
		simulation.frame_index++;
		simulation.frame_start_time = Clock::now();
	}

	void WorkerUpdate(Simulation& simulation, size_t index)
	{

	}
}