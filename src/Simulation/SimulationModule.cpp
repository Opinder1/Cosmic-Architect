#include "SimulationModule.h"
#include "SimulationComponents.h"

#include <flecs/flecs.h>

namespace voxel_game
{
	SimulationModule::SimulationModule(flecs::world& world)
	{
		world.module<SimulationModule>();

		world.import<SimulationComponents>();

		world.system<SimulationTime>("WorldUpdateTime")
			.kind(flecs::OnUpdate)
			.term_at(1).src<SimulationTime>()
			.each([](SimulationTime& world_time)
		{
			world_time.frame_index++;
			world_time.frame_start = Clock::now();
		});
	}
}