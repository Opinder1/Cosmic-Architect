#include "EntityLoader.h"

namespace
{
	std::string_view UUIDToData(const UUID& id)
	{
		return std::string_view((const char*)&id, sizeof(UUID));
	}
}

namespace voxel_game::loading
{
	const size_t k_num_database_workers = 4;
	const size_t k_entity_pool_max = 1024;

	EntityLoader::EntityLoader() :
		m_database_async(&m_database, k_num_database_workers)
	{}

	EntityLoader::~EntityLoader()
	{
		m_running.store(false, std::memory_order_release);

		if (m_thread.joinable())
		{
			m_thread.join();
		}

#if defined(DEBUG_ENABLED)
		m_modification_stage.SetThreads(std::this_thread::get_id(), std::this_thread::get_id());
#endif

		// Merge remaining modifications that were already published
		m_modification_stage.Retrieve().merge();

		// Merge remaining modifications that were not published
		m_modification_stage.Publish();
		m_modification_stage.Retrieve().merge();
	}


	void EntityLoader::Initialize(flecs::world& world)
	{
		if (m_running.load(std::memory_order_acquire))
		{
			DEBUG_PRINT_WARN("Tried to initialize when already running");
			return;
		}

		m_world = world.c_ptr();

		m_modification_stage.Reset(world.async_stage(), world.async_stage(), world.async_stage());

		m_running.store(true, std::memory_order_release);

		m_thread = std::thread(&EntityLoader::ThreadLoop, this, std::this_thread::get_id());
	}

	void EntityLoader::Progress()
	{
		m_load_commands.PublishCommands();
		m_save_commands.PublishCommands();
		m_delete_commands.PublishCommands();

		m_modification_stage.Retrieve().merge();

		while (m_entity_pool.size() < k_entity_pool_max)
		{
			m_entity_pool.push_back(flecs::entity(m_world));
		}
	}

	void EntityLoader::ThreadLoop(std::thread::id owner_id)
	{
#if defined(DEBUG_ENABLED)
		std::thread::id thread_id = std::this_thread::get_id();
		m_load_commands.SetThreads(thread_id, owner_id);
		m_save_commands.SetThreads(thread_id, owner_id);
		m_delete_commands.SetThreads(thread_id, owner_id);
		m_modification_stage.SetThreads(owner_id, thread_id);
#endif

		while (m_running.load(std::memory_order_acquire))
		{
			m_modifications_added = false;

			ProcessLoadTasks();
			ProcessSaveTasks();
			ProcessDeleteTasks();

			if (m_modifications_added)
			{
				m_modification_stage.Publish();
			}
		}

#if defined(DEBUG_ENABLED)
		m_modification_stage.SetThreads(std::thread::id{}, std::thread::id{});
		m_delete_commands.SetThreads(std::thread::id{}, std::thread::id{});
		m_save_commands.SetThreads(std::thread::id{}, std::thread::id{});
		m_load_commands.SetThreads(std::thread::id{}, std::thread::id{});
#endif
	}

	void EntityLoader::LoadEntity(flecs::entity_t entity, UUID uuid)
	{
		m_load_commands.AddCommand({ entity, uuid });
	}

	void EntityLoader::SaveEntity(flecs::entity_t entity, EntitySaveData&& data)
	{
		m_save_commands.AddCommand({ entity, std::move(data) });
	}

	void EntityLoader::DeleteEntity(flecs::entity_t entity)
	{
		m_delete_commands.AddCommand({ entity });
	}

	void EntityLoader::ProcessLoadTasks()
	{
		std::vector<LoadTask> load_commands;

		m_load_commands.RetrieveCommands(load_commands);

		for (LoadTask& task : load_commands)
		{
			task.future = m_database_async.Get(UUIDToData(task.uuid));

			m_load_tasks.emplace_back(std::move(task));
		}

		load_commands.clear();

		auto task_end = m_load_tasks.end();
		for (auto task_it = m_load_tasks.begin(); task_it != task_end;)
		{
			if (!task_it->future.valid())
			{
				task_it++;
				continue;
			}

			tkrzw::Status status;
			std::string data;
			std::tie(status, data) = std::move(task_it->future.get());

			flecs::entity e;
			e.from_json(data.c_str());

			m_modifications_added = true;

			// Unordered erase
			task_it = m_load_tasks.erase(task_it);
		}
	}

	void EntityLoader::ProcessSaveTasks()
	{
		std::vector<SaveTask> save_commands;

		m_save_commands.RetrieveCommands(save_commands);

		for (SaveTask& task : save_commands)
		{
			m_save_tasks.emplace_back(std::move(task));
		}

		save_commands.clear();

		auto task_end = m_save_tasks.end();
		for (auto task_it = m_save_tasks.begin(); task_it != task_end;)
		{
			task_it++;
		}
	}

	void EntityLoader::ProcessDeleteTasks()
	{
		std::vector<DeleteTask> delete_commands;

		m_delete_commands.RetrieveCommands(delete_commands);

		for (DeleteTask& task : delete_commands)
		{
			m_delete_tasks.emplace_back(std::move(task));
		}

		delete_commands.clear();

		auto task_end = m_delete_tasks.end();
		for (auto task_it = m_delete_tasks.begin(); task_it != task_end;)
		{
			task_it++;
		}
	}
}