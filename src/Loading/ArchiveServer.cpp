#include "ArchiveServer.h"

/*
namespace
{
	std::string_view UUIDToData(const UUID& id)
	{
		return std::string_view((const char*)&id, sizeof(UUID));
	}
}

namespace voxel_game::loading
{
	const size_t k_entity_pool_max = 1024;

	ArchiveServer::ArchiveServer()
	{}

	ArchiveServer::~ArchiveServer()
	{
		m_running.store(false, std::memory_order_release);

		if (m_thread.joinable()) // The worker thread is using our memory so make sure its stopped before we are deleted
		{
			m_thread.join();

			// Merge remaining modifications
			m_modifications_swap.Retrieve(m_modifications_read);
			m_modifications_read.merge();
			m_modifications_write.merge();
		}

	}


	void ArchiveServer::Initialize(flecs::world& world, ThreadMode thread_mode)
	{
		if (m_running.load(std::memory_order_acquire))
		{
			DEBUG_PRINT_WARN("Tried to initialize when already running");
			return;
		}

#if defined(DEBUG_ENABLED)
			m_owner_id = std::this_thread::get_id();
#endif

		m_worker.world = world.c_ptr();

		m_running.store(true, std::memory_order_release);

		if (thread_mode == ThreadMode::MultiThreaded)
		{
			m_modifications_write = world.async_stage();
			m_modifications_swap.Reset(m_modifications_read.async_stage());
			m_modifications_read = world.async_stage();

			m_thread = std::thread(&ArchiveServer::ThreadLoop, this);

		}
	}

	bool ArchiveServer::IsThreaded()
	{
		return m_thread.joinable();
	}

	void ArchiveServer::Progress()
	{
		DEBUG_ASSERT(m_running, "Our worker thread should be running");
		DEBUG_ASSERT(m_owner_id == std::this_thread::get_id(), "Progress() should be called by the owner thread");

		if (IsThreaded())
		{
			m_commands_swap.Publish(m_commands_write);

			m_modifications_swap.Retrieve(m_modifications_read);
			m_modifications_read.merge();

			while (m_worker.entity_pool.size() < k_entity_pool_max)
			{
				m_worker.entity_pool.push_back(flecs::entity(m_worker.world));
			}
		}
		else
		{
			ProcessTasks();
		}
	}

	ArchiveServer::DBHandle ArchiveServer::OpenArchive(const godot::StringName& path)
	{
		DEBUG_ASSERT(m_running, "Our worker thread should be running");
		DEBUG_ASSERT(!IsArchiveOpen(path), "The database is already open");

		if (IsThreaded())
		{
			m_commands_write.AddCommand<&ArchiveServer::DoOpenCommand>(path, m_new_handle);
		}
		else
		{
			DoOpenCommand(path, m_new_handle);
		}

		m_open_databases.emplace(path, m_new_handle);

		return m_new_handle++;
	}

	void ArchiveServer::CloseArchive(const godot::StringName& path)
	{
		DEBUG_ASSERT(m_running, "Our worker thread should be running");

		auto it = m_open_databases.find(path);

		DEBUG_ASSERT(it != m_open_databases.end(), "The database is already open");

		if (IsThreaded())
		{
			m_commands_write.AddCommand<&ArchiveServer::DoCloseCommand>(it->second);
		}
		else
		{
			DoCloseCommand(it->second);
		}

		m_open_databases.erase(it);
	}

	bool ArchiveServer::IsArchiveOpen(const godot::StringName& path)
	{
		DEBUG_ASSERT(m_running, "Our worker thread should be running");
		return m_open_databases.find(path) != m_open_databases.end();
	}

	void ArchiveServer::LoadEntity(UUID uuid, flecs::entity_t entity, DBHandle db_handle)
	{
		DEBUG_ASSERT(m_running, "Our worker thread should be running");

		if (IsThreaded())
		{
			m_commands_write.AddCommand<&ArchiveServer::DoLoadCommand>(uuid, entity, db_handle);
		}
		else
		{
			DoLoadCommand(uuid, entity, db_handle);
		}
	}

	void ArchiveServer::SaveEntity(UUID uuid, flecs::entity_t entity, DBHandle db_handle, EntitySaveData&& data)
	{
		DEBUG_ASSERT(m_running, "Our worker thread should be running");

		if (IsThreaded())
		{
			m_commands_write.AddCommand<&ArchiveServer::DoSaveCommand>(uuid, entity, db_handle, std::move(data));
		}
		else
		{
			DoSaveCommand(uuid, entity, db_handle, std::move(data));
		}
	}

	void ArchiveServer::DeleteEntity(UUID uuid, flecs::entity_t entity, DBHandle db_handle)
	{
		DEBUG_ASSERT(m_running, "Our worker thread should be running");

		if (IsThreaded())
		{
			m_commands_write.AddCommand<&ArchiveServer::DoDeleteCommand>(uuid, entity, db_handle);
		}
		else
		{
			DoDeleteCommand(uuid, entity, db_handle);
		}
	}

	bool ArchiveServer::HasTasks()
	{
		return !m_worker.load_tasks.empty() || !m_worker.save_tasks.empty() || !m_worker.delete_tasks.empty();
	}

	void ArchiveServer::DoOpenCommand(godot::StringName path, DBHandle db_handle)
	{
		auto&& [it, emplaced] = m_worker.databases.try_emplace(db_handle);

		DEBUG_ASSERT(emplaced, "Database already open");

		Database& database = it->second;

		database.path = path;

		database.database.Open(std::string(database.path.utf8()), true);
	}

	void ArchiveServer::DoCloseCommand(DBHandle db_handle)
	{
		auto it = m_worker.databases.find(db_handle);

		DEBUG_ASSERT(it != m_worker.databases.end(), "Database handle was not found");

		Database& database = it->second;

		DEBUG_ASSERT(database.entities.size() == 0, "All entities should have been saved and deleted");
		
		m_worker.databases.erase(it);
	}

	void ArchiveServer::DoLoadCommand(UUID uuid, flecs::entity_t entity, DBHandle db_handle)
	{
		EntityData& entity_data = m_worker.entity_cache[uuid];

		if (entity_data.refcount > 0)
		{
			return;
		}

		entity_data.entity = entity;

		m_worker.load_tasks.push_back(LoadTask{ uuid, &entity_data });
	}

	void ArchiveServer::DoSaveCommand(UUID uuid, flecs::entity_t entity, DBHandle database, EntitySaveData&& data)
	{

	}

	void ArchiveServer::DoDeleteCommand(UUID uuid, flecs::entity_t entity, DBHandle database)
	{

	}

	void ArchiveServer::ProcessCommands()
	{
		TCommandBuffer<ArchiveServer> commands_read;

		m_commands_swap.Retrieve(commands_read);

		commands_read.ProcessCommands(*this);
	}

	bool ArchiveServer::ProcessLoadTask(LoadTask& task)
	{
		m_worker.modifications_added = true;

		return true;
	}

	bool ArchiveServer::ProcessSaveTask(SaveTask& task)
	{
		return true;
	}

	bool ArchiveServer::ProcessDeleteTask(DeleteTask& task)
	{
		return true;
	}

	void ArchiveServer::ProcessTasks()
	{
		for (auto task_it = m_worker.load_tasks.begin(); task_it != m_worker.load_tasks.end();)
		{
			if (ProcessLoadTask(*task_it))
			{
				// Unordered erase
				task_it = m_worker.load_tasks.erase(task_it);
			}
			else
			{
				task_it++;
			}
		}

		for (auto task_it = m_worker.save_tasks.begin(); task_it != m_worker.save_tasks.end();)
		{
			if (ProcessSaveTask(*task_it))
			{
				// Unordered erase
				task_it = m_worker.save_tasks.erase(task_it);
			}
			else
			{
				task_it++;
			}
		}

		for (auto task_it = m_worker.delete_tasks.begin(); task_it != m_worker.delete_tasks.end();)
		{
			if (ProcessDeleteTask(*task_it))
			{
				// Unordered erase
				task_it = m_worker.delete_tasks.erase(task_it);
			}
			else
			{
				task_it++;
			}
		}
	}

	void ArchiveServer::ThreadLoop()
	{
		while (m_running.load(std::memory_order_acquire) && !HasTasks())
		{
			m_worker.modifications_added = false;

			ProcessCommands();

			ProcessTasks();

			if (m_worker.modifications_added)
			{
				m_modifications_swap.Publish(m_modifications_write);
			}

			std::this_thread::yield();
		}
	}
}
*/