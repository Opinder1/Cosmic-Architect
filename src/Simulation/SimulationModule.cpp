#include "SimulationModule.h"
#include "SimulationComponents.h"

#include <flecs/flecs.h>

namespace voxel_game::sim
{
	Module::Module(flecs::world& world)
	{
		world.module<Module>();

		world.import<Components>();

		world.add<GlobalTime>();

		world.add<Config>();

		world.add<ThreadEntityPools>();

		world.system<GlobalTime>("WorldUpdateTime")
			.kind(flecs::OnUpdate)
			.term_at(0).src<GlobalTime>()
			.each([](GlobalTime& world_time)
		{
			world_time.frame_index++;
			world_time.frame_start = Clock::now();
		});

		world.system<ThreadEntityPools>("ProcessEntityThreadCommands")
			.kind(flecs::OnUpdate)
			.term_at(0).src<ThreadEntityPools>()
			.each([world = world.c_ptr()](ThreadEntityPools& thread_pools)
		{
			for (ThreadEntityPool& thread : thread_pools.threads)
			{
				thread.AllocateEntities(world);
			}
		});
	}
}