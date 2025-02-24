#include "SimulationModule.h"
#include "SimulationComponents.h"

#include <flecs/flecs.h>

namespace voxel_game::sim
{
	thread_local ThreadEntityPool* thread_pool = nullptr;

	ThreadEntityPool& GetPool()
	{
		DEBUG_ASSERT(thread_pool != nullptr, "This thread doesn't have a pool set");
		return *thread_pool;
	}

	Module::Module(flecs::world& world)
	{
		world.module<Module>();

		world.import<Components>();

		world.add<CFrame>();

		world.add<CConfig>();

		world.add<CEntityPools>();

		world.system<CFrame>("WorldUpdateTime")
			.kind(flecs::OnUpdate)
			.term_at(0).src<CFrame>()
			.each([](CFrame& world_time)
		{
			world_time.frame_index++;
			world_time.frame_start_time = Clock::now();
		});

		world.system<CEntityPools>("ProcessEntityThreadCommands")
			.kind(flecs::OnUpdate)
			.term_at(0).src<CEntityPools>()
			.each([](CEntityPools& thread_pools)
		{
			for (ThreadEntityPool& thread : thread_pools.threads)
			{
				thread.AllocateEntities();
			}
		});

		world.system<const sim::CThreadWorker, CEntityPools>(DEBUG_ONLY("SetThreadContexts"))
			.multi_threaded()
			.term_at(1).src<CEntityPools>()
			.each([](flecs::iter& it, size_t i, const sim::CThreadWorker& worker, CEntityPools& pools)
		{
			thread_pool = &pools.threads[it.world().get_stage_id()];
		});
	}
}