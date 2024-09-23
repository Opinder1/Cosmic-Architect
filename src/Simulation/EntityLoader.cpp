#include "EntityLoader.h"

namespace voxel_game
{
	const size_t k_entity_pool_max = 1024;

	EntityLoader::EntityLoader(const flecs::world& world) :
		m_world(world),
		m_modification_stage(world.async_stage(), world.async_stage(), world.async_stage())
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
			std::vector<Command> commands;

			m_commands.RetrieveCommands(commands);

			m_modifications_added = false;

			for (const Command& command : commands)
			{
				switch (command.type)
				{
				case CommandType::LoadEntity:
					LoadEntity(command.uuid);
					break;
				}
			}

			if (m_modifications_added)
			{
				m_modification_stage.Publish();
			}
		}
	}

	void EntityLoader::LoadEntity(UUID id)
	{

	}
}