#include "EntityLoader.h"

namespace
{
	std::string_view UUIDToData(const UUID& id)
	{
		return std::string_view((const char*)&id, sizeof(UUID));
	}
}

namespace voxel_game
{
	const size_t k_num_database_workers = 4;
	const size_t k_entity_pool_max = 1024;

	EntityLoader::EntityLoader(flecs::world& world) :
		m_world(world),
		m_modification_stage(world.async_stage(), world.async_stage(), world.async_stage()),
		m_database_async(&m_database, k_num_database_workers)
	{
		m_running.store(true, std::memory_order_release);

		if (!m_thread.joinable())
		{
			m_thread = std::thread(&EntityLoader::ThreadLoop, this);
		}
	}

	EntityLoader::~EntityLoader()
	{
		m_running.store(false, std::memory_order_release);

		if (m_thread.joinable())
		{
			m_thread.join();
		}
	}

	void EntityLoader::Progress()
	{
		m_modification_stage.Retrieve().merge();

		m_commands.PublishCommands();

		while (m_entity_pool.size() < k_entity_pool_max)
		{
			m_entity_pool.push_back(flecs::entity(m_world));
		}
	}

	void EntityLoader::ThreadLoop()
	{
		while (m_running.load(std::memory_order_acquire))
		{
			ProcessCommands();

			m_modifications_added = false;

			ProcessCreateTasks();
			ProcessDeleteTasks();
			ProcessLoadTasks();
			ProcessUnloadTasks();
			ProcessReloadTasks();
			ProcessSaveTasks();
			ProcessSaveAndUnloadTasks();

			if (m_modifications_added)
			{
				m_modification_stage.Publish();
			}
		}
	}

	void EntityLoader::ProcessCommands()
	{
		std::vector<Command> commands;

		m_commands.RetrieveCommands(commands);

		for (const Command& command : commands)
		{
			switch (command.type)
			{
			case CommandType::CreateEntity:
				CreateEntity(command.uuid);
				break;

			case CommandType::DeleteEntity:
				DeleteEntity(command.uuid);
				break;

			case CommandType::LoadEntity:
				LoadEntity(command.uuid);
				break;

			case CommandType::UnloadEntity:
				UnloadEntity(command.uuid);
				break;

			case CommandType::ReloadEntity:
				ReloadEntity(command.uuid);
				break;

			case CommandType::SaveEntity:
				SaveEntity(command.uuid);
				break;

			case CommandType::SaveAndUnloadEntity:
				SaveAndUnloadEntity(command.uuid);
				break;
			}
		}
	}

	void EntityLoader::CreateEntity(UUID id)
	{

	}

	void EntityLoader::DeleteEntity(UUID id)
	{

	}

	void EntityLoader::LoadEntity(UUID id)
	{
		LoadTask& task = m_load_tasks.emplace_back();

		task.id = id;
		task.future = m_database_async.Get(UUIDToData(id));
	}

	void EntityLoader::UnloadEntity(UUID id)
	{

	}

	void EntityLoader::ReloadEntity(UUID id)
	{
		UnloadEntity(id);
		LoadEntity(id);
	}

	void EntityLoader::SaveEntity(UUID id)
	{

	}

	void EntityLoader::SaveAndUnloadEntity(UUID id)
	{
		SaveEntity(id);
		UnloadEntity(id);
	}

	void EntityLoader::ProcessCreateTasks()
	{
		auto end = m_create_tasks.end();
		for (auto it = m_create_tasks.begin(); it != end;)
		{

		}
	}

	void EntityLoader::ProcessDeleteTasks()
	{
		auto end = m_delete_tasks.end();
		for (auto it = m_delete_tasks.begin(); it != end;)
		{

		}
	}

	void EntityLoader::ProcessLoadTasks()
	{
		auto end = m_load_tasks.end();
		for (auto it = m_load_tasks.begin(); it != end;)
		{
			if (!it->future.valid())
			{
				it++;
			}

			tkrzw::Status status;
			std::string data;
			std::tie(status, data) = std::move(it->future.get());

			flecs::entity e;
			e.from_json(data.c_str());

			m_modifications_added = true;

			// Unordered erase
			m_load_tasks.erase(it);
		}
	}

	void EntityLoader::ProcessUnloadTasks()
	{
		auto end = m_unload_tasks.end();
		for (auto it = m_unload_tasks.begin(); it != end;)
		{

		}
	}

	void EntityLoader::ProcessReloadTasks()
	{
		auto end = m_reload_tasks.end();
		for (auto it = m_reload_tasks.begin(); it != end;)
		{

		}
	}

	void EntityLoader::ProcessSaveTasks()
	{
		auto end = m_save_tasks.end();
		for (auto it = m_save_tasks.begin(); it != end;)
		{

		}
	}

	void EntityLoader::ProcessSaveAndUnloadTasks()
	{
		auto end = m_save_and_unload_tasks.end();
		for (auto it = m_save_and_unload_tasks.begin(); it != end;)
		{

		}
	}
}