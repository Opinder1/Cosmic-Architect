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

	EntityLoader::EntityLoader(const flecs::world& world) :
		m_world(world),
		m_modification_stage(world.async_stage(), world.async_stage(), world.async_stage()),
		m_database_async(&m_database, k_num_database_workers)
	{
		m_running.store(true, std::memory_order_release);
		m_thread = std::thread(&EntityLoader::ThreadLoop, this);
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
			m_entity_pool.push_back(m_world.entity());
		}
	}

	void EntityLoader::ThreadLoop()
	{
		while (m_running.load(std::memory_order_acquire))
		{
			ProcessCommands();

			m_modifications_added = false;

			ProcessLoadTasks();

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
			case CommandType::LoadEntity:
				LoadEntity(command.uuid);
				break;
			}
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
			*it = std::move(m_load_tasks.back());
			m_load_tasks.pop_back();
		}
	}

	void EntityLoader::LoadEntity(UUID id)
	{
		LoadTask& task = m_load_tasks.emplace_back();

		task.id = id;
		task.future = m_database_async.Get(UUIDToData(id));
	}
}