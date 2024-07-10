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

		world.system<ThreadEntityPools>("ProcessEntityThreadCommands")
			.kind(flecs::OnUpdate)
			.term_at(1).src<ThreadEntityPools>()
			.each([&world](ThreadEntityPools& thread_pools)
		{
			for (ThreadEntityPool& thread : thread_pools.threads)
			{
				size_t new_required = 1024 - thread.new_entities.size();

				if (new_required == 1024)
				{
					thread.new_entities.reserve(1024);
				}

				for (size_t i = 0; i < new_required; i++)
				{
					thread.new_entities.push_back(world.entity());
				}
			}
		});
	}
}