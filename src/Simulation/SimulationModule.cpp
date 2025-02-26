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

		for (size_t worker_index = 0; worker_index < world.get_stage_count(); worker_index++)
		{
			flecs::entity thread_worker = world.entity();

#if defined(DEBUG_ENABLED)
			thread_worker.set_name(godot::vformat("Worker%d", worker_index).utf8());
#endif

			thread_worker.ensure<CThreadWorker>().index = worker_index;
		}

		world.system<CFrame>("WorldUpdateTime")
			.kind(flecs::OnUpdate)
			.term_at(0).singleton()
			.each([](CFrame& world_time)
		{
			world_time.frame_index++;
			world_time.frame_start_time = Clock::now();
		});

		world.system<CEntityPools>("ProcessEntityThreadCommands")
			.kind(flecs::OnUpdate)
			.term_at(0).singleton()
			.each([](CEntityPools& thread_pools)
		{
			for (ThreadEntityPool& thread : thread_pools.threads)
			{
				thread.AllocateEntities();
			}
		});

		world.system<const CThreadWorker, CEntityPools>(DEBUG_ONLY("SetThreadContexts"))
			.multi_threaded()
			.term_at(1).singleton()
			.each([](flecs::iter& it, size_t i, const CThreadWorker& worker, CEntityPools& pools)
		{
			thread_pool = &pools.threads[it.world().get_stage_id()];
		});
	}
}