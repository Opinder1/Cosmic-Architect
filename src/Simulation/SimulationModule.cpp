#include "SimulationModule.h"
#include "SimulationComponents.h"

#include <flecs/flecs.h>

namespace voxel_game
{
	void AdvanceSimulationGlobal(SimulationGlobal& world_time)
	{
		world_time.frame_index++;
		world_time.frame_start = Clock::now();
	}

	SimulationModule::SimulationModule(flecs::world& world)
	{
		world.module<SimulationModule>();

		world.import<SimulationComponents>();

		world.system<SimulationGlobal>("WorldUpdateTime")
			.kind(flecs::OnUpdate)
			.term_at(1).src<SimulationGlobal>()
			.each(AdvanceSimulationGlobal);
	}
}