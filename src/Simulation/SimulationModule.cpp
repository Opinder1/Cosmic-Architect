#include "SimulationModule.h"
#include "SimulationComponents.h"

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/json.hpp>

#include <flecs/flecs.h>

namespace voxel_game::sim
{
	thread_local ThreadEntityPool* thread_pool = nullptr;

	ThreadEntityPool& GetPool()
	{
		DEBUG_ASSERT(thread_pool != nullptr, "This thread doesn't have a pool set");
		return *thread_pool;
	}

	void LoadJsonConfig(CConfig& config)
	{
		if (config.path.is_empty())
		{
			return;
		}

		godot::Ref<godot::FileAccess> file = godot::FileAccess::open(config.path, godot::FileAccess::READ);

		if (file.is_null() || file->get_error() != godot::Error::OK)
		{
			DEBUG_PRINT_ERROR(godot::vformat("Failed to open the file %s", config.path));
			return;
		}

		config.values = godot::JSON::parse_string(file->get_as_text());

		file->close();
	}

	void SaveJsonConfig(CConfig& config)
	{
		if (config.path.is_empty())
		{
			return;
		}

		godot::Ref<godot::FileAccess> file = godot::FileAccess::open(config.path, godot::FileAccess::WRITE);

		if (file.is_null() || file->get_error() != godot::Error::OK)
		{
			DEBUG_PRINT_ERROR(godot::vformat("Failed to open the file %s", config.path));
			return;
		}

		file->store_string(godot::JSON::stringify(config.values, "    "));

		file->close();
	}

	void InitializeConfig(flecs::entity entity, const godot::String& path, const ConfigDefaults& defaults)
	{
		sim::CConfig& config = entity.ensure<sim::CConfig>();

		config.path = path;

		sim::LoadJsonConfig(config);

		for (auto&& [key, value] : defaults)
		{
			if (!config.values.has(key))
			{
				config.values[key] = value;
			}
		}

		sim::SaveJsonConfig(config);
	}

	Module::Module(flecs::world& world)
	{
		world.module<Module>();

		world.import<Components>();

		world.component<CEntityPools>()
			.on_remove([world = world.c_ptr()](CEntityPools& pools)
		{
			for (ThreadEntityPool& pool : pools.threads)
			{
				pool.ClearEntities(world);
			}
		});

		world.add<CFrame>();

		world.add<CEntityPools>();

		for (size_t worker_index = 0; worker_index < world.get_stage_count(); worker_index++)
		{
			flecs::entity thread_worker = world.entity();

#if defined(DEBUG_ENABLED)
			thread_worker.set_name(godot::vformat("Worker%d", worker_index).utf8());
#endif

			thread_worker.ensure<CThreadWorker>().index = worker_index;
		}

		world.observer<CConfig>("ConfigUpdate")
			.event(flecs::OnSet)
			.event(flecs::OnRemove)
			.each([](CConfig& config)
		{
			SaveJsonConfig(config);
		});

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
			.each([world = world.c_ptr()](CEntityPools& thread_pools)
		{
			for (ThreadEntityPool& thread : thread_pools.threads)
			{
				thread.AllocateEntities(world);
			}
		});

		world.system<const CThreadWorker, CEntityPools>(DEBUG_ONLY("SetThreadContexts"))
			.multi_threaded()
			.term_at(1).singleton()
			.each([](flecs::iter& it, size_t i, const CThreadWorker& worker, CEntityPools& pools)
		{
			thread_pool = &pools.threads[it.world().get_stage_id()];
			thread_pool->SetStage(it.world());
		});
	}
}