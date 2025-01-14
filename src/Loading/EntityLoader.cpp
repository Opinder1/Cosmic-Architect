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
	const size_t k_entity_pool_max = 1024;

	EntityLoader::EntityLoader()
	{}

	EntityLoader::~EntityLoader()
	{
		if (m_thread.joinable()) // The worker thread is using our memory so make sure its stopped before we are deleted
		{
			m_running.store(false, std::memory_order_release);

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

		m_worker.world = world.c_ptr();

		m_modification_stage.Reset(world.async_stage(), world.async_stage(), world.async_stage());

		m_running.store(true, std::memory_order_release);

		m_thread = std::thread(&EntityLoader::ThreadLoop, this);

#if defined(DEBUG_ENABLED)
		std::thread::id owner_id = std::this_thread::get_id();
		m_open_commands.SetThreads(m_thread.get_id(), owner_id);
		m_close_commands.SetThreads(m_thread.get_id(), owner_id);
		m_load_commands.SetThreads(m_thread.get_id(), owner_id);
		m_save_commands.SetThreads(m_thread.get_id(), owner_id);
		m_delete_commands.SetThreads(m_thread.get_id(), owner_id);
		m_modification_stage.SetThreads(owner_id, m_thread.get_id());
#endif
	}

	void EntityLoader::Progress()
	{
		DEBUG_ASSERT(m_thread.joinable() && m_running, "Our worker thread should be running");

		m_open_commands.PublishCommands();
		m_close_commands.PublishCommands();
		m_load_commands.PublishCommands();
		m_save_commands.PublishCommands();
		m_delete_commands.PublishCommands();

		m_modification_stage.Retrieve().merge();

		while (m_worker.entity_pool.size() < k_entity_pool_max)
		{
			m_worker.entity_pool.push_back(flecs::entity(m_worker.world));
		}
	}

	EntityLoader::DBHandle EntityLoader::OpenArchive(const godot::StringName& path)
	{
		DEBUG_ASSERT(m_thread.joinable() && m_running, "Our worker thread should be running");
		DEBUG_ASSERT(!IsArchiveOpen(path), "The database is already open");

		m_open_commands.AddCommand({ path, m_new_handle });

		m_open_databases.emplace(path, m_new_handle);

		return m_new_handle++;
	}

	void EntityLoader::CloseArchive(const godot::StringName& path)
	{
		DEBUG_ASSERT(m_thread.joinable() && m_running, "Our worker thread should be running");

		auto it = m_open_databases.find(path);

		DEBUG_ASSERT(it != m_open_databases.end(), "The database is already open");

		m_close_commands.AddCommand({ it->second });

		m_open_databases.erase(it);
	}

	bool EntityLoader::IsArchiveOpen(const godot::StringName& path)
	{
		return m_open_databases.find(path) != m_open_databases.end();
	}

	void EntityLoader::AddComponent(uint16_t id, flecs::entity_t entity, ComponentRead read, ComponentWrite write)
	{

	}

	void EntityLoader::LoadEntity(UUID uuid, flecs::entity_t entity, DBHandle database)
	{
		DEBUG_ASSERT(m_thread.joinable() && m_running, "Our worker thread should be running");

		m_load_commands.AddCommand({ uuid, entity, database });
	}

	void EntityLoader::SaveEntity(UUID uuid, flecs::entity_t entity, DBHandle database, EntitySaveData&& data)
	{
		DEBUG_ASSERT(m_thread.joinable() && m_running, "Our worker thread should be running");

		m_save_commands.AddCommand({ uuid, entity, database, std::move(data) });
	}

	void EntityLoader::DeleteEntity(UUID uuid, flecs::entity_t entity, DBHandle database)
	{
		DEBUG_ASSERT(m_thread.joinable() && m_running, "Our worker thread should be running");

		m_delete_commands.AddCommand({ uuid, entity, database });
	}

	bool EntityLoader::HasTasks()
	{
		return !m_worker.load_tasks.empty() || !m_worker.save_tasks.empty() || !m_worker.delete_tasks.empty();
	}

	void EntityLoader::DoOpenCommand(OpenCommand& command)
	{
		auto&& [it, emplaced] = m_worker.databases.try_emplace(command.handle);

		DEBUG_ASSERT(emplaced, "Database already open");

		Database& database = it->second;

		database.path = command.path;

		database.database.Open(std::string(database.path.utf8()), true);
	}

	void EntityLoader::DoCloseCommand(CloseCommand& command)
	{
		auto it = m_worker.databases.find(command.handle);

		DEBUG_ASSERT(it != m_worker.databases.end(), "Database handle was not found");

		Database& database = it->second;

		DEBUG_ASSERT(database.entities.size() == 0, "All entities should have been saved and deleted");
		
		m_worker.databases.erase(it);
	}

	void EntityLoader::DoLoadCommand(LoadCommand& command)
	{
		Entity& entity = m_worker.entity_cache[command.uuid];

		if (entity.refcount > 0)
		{
			return;
		}

		entity.entity = command.entity;

		m_worker.load_tasks.push_back(LoadTask{ command.uuid, &entity });
	}

	void EntityLoader::DoSaveCommand(SaveCommand& command)
	{

	}

	void EntityLoader::DoDeleteCommand(DeleteCommand& command)
	{

	}

	void EntityLoader::ProcessCommands()
	{
		{
			std::vector<OpenCommand> commands;

			m_open_commands.RetrieveCommands(commands);

			for (OpenCommand& command : commands)
			{
				DoOpenCommand(command);
			}
		}

		{
			std::vector<CloseCommand> commands;

			m_close_commands.RetrieveCommands(commands);

			for (CloseCommand& command : commands)
			{
				DoCloseCommand(command);
			}
		}

		{
			std::vector<LoadCommand> commands;

			m_load_commands.RetrieveCommands(commands);

			for (LoadCommand& command : commands)
			{
				DoLoadCommand(command);
			}
		}
		
		{
			std::vector<SaveCommand> commands;

			m_save_commands.RetrieveCommands(commands);

			for (SaveCommand& command : commands)
			{
				DoSaveCommand(command);
			}
		}

		{
			std::vector<DeleteCommand> commands;

			m_delete_commands.RetrieveCommands(commands);

			for (DeleteCommand& command : commands)
			{
				DoDeleteCommand(command);
			}
		}
	}

	bool EntityLoader::ProcessLoadTask(Entity& entity)
	{
		m_worker.modifications_added = true;

		return true;
	}

	void EntityLoader::ProcessTasks()
	{
		for (auto task_it = m_worker.load_tasks.begin(); task_it != m_worker.load_tasks.end();)
		{
			if (ProcessLoadTask(*task_it->entity))
			{
				// Unordered erase
				task_it = m_worker.load_tasks.erase(task_it);
			}
			else
			{
				task_it++;
			}
		}
	}

	void EntityLoader::ThreadLoop()
	{
		while (m_running.load(std::memory_order_acquire) && !HasTasks())
		{
			m_worker.modifications_added = false;

			ProcessCommands();

			ProcessTasks();

			if (m_worker.modifications_added)
			{
				m_modification_stage.Publish();
			}

			std::this_thread::yield();
		}
	}
}