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

	void EntityLoader::CreateEntity(flecs::entity_t target, UUID schematic)
	{
		m_commands.AddCommand(Command{ CommandType::CreateEntity, target, schematic });
	}

	void EntityLoader::LoadEntity(UUID uuid)
	{
		m_commands.AddCommand(Command{ CommandType::LoadEntity, 0, uuid });
	}

	void EntityLoader::SaveEntity(flecs::entity_t entity)
	{
		m_commands.AddCommand(Command{ CommandType::SaveEntity, entity });
	}

	void EntityLoader::DeleteEntity(flecs::entity_t entity)
	{
		m_commands.AddCommand(Command{ CommandType::DeleteEntity, entity });
	}

	void EntityLoader::ThreadLoop()
	{
		while (m_running.load(std::memory_order_acquire))
		{
			ProcessCommands();

			m_modifications_added = false;

			ProcessCreateTasks();
			ProcessLoadTasks();
			ProcessSaveTasks();
			ProcessDeleteTasks();

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
				DoCreateEntity(command.entity, command.uuid);
				break;

			case CommandType::LoadEntity:
				DoLoadEntity(command.uuid);
				break;

			case CommandType::SaveEntity:
				DoSaveEntity(command.entity);
				break;

			case CommandType::DeleteEntity:
				DoDeleteEntity(command.entity);
				break;
			}
		}
	}

	void EntityLoader::DoCreateEntity(flecs::entity_t target, UUID schematic)
	{
		auto it = m_entity_cache.find(schematic);

		if (it == m_entity_cache.end())
		{
			DoLoadEntity(schematic);
		}

		CreateTask& task = m_create_tasks.emplace_back();

		task.target = target;
		task.schematic = schematic;
	}

	void EntityLoader::DoLoadEntity(UUID uuid)
	{
		LoadTask& task = m_load_tasks.emplace_back();

		task.id = uuid;
		task.future = m_database_async.Get(UUIDToData(uuid));
	}

	void EntityLoader::DoSaveEntity(flecs::entity_t entity)
	{

	}

	void EntityLoader::DoDeleteEntity(flecs::entity_t entity)
	{

	}

	void EntityLoader::ProcessCreateTasks()
	{
		auto task_end = m_create_tasks.end();
		for (auto task_it = m_create_tasks.begin(); task_it != task_end;)
		{
			auto cache_it = m_entity_cache.find(task_it->schematic);

			if (cache_it == m_entity_cache.end())
			{
				task_it++;
				continue;
			}
			
			flecs::entity target = m_modification_stage.Write().entity(task_it->target);

			target.add(flecs::OneOf, cache_it->second.entity);

			// Unordered erase
			task_it = m_create_tasks.erase(task_it);
		}
	}

	void EntityLoader::ProcessDeleteTasks()
	{
		auto task_end = m_delete_tasks.end();
		for (auto task_it = m_delete_tasks.begin(); task_it != task_end;)
		{
			task_it++;
		}
	}

	void EntityLoader::ProcessSaveTasks()
	{
		auto task_end = m_save_tasks.end();
		for (auto task_it = m_save_tasks.begin(); task_it != task_end;)
		{
			task_it++;
		}
	}

	void EntityLoader::ProcessLoadTasks()
	{
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
}