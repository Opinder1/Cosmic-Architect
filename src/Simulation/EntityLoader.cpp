#include "EntityLoader.h"

namespace voxel_game
{
	const size_t k_entity_pool_max = 1024;

	EntityLoader::EntityLoader(const flecs::world& world)
	{
		m_world = world;

		m_read_stage = world.async_stage();
		m_swap_stage = world.async_stage();
		m_write_stage = world.async_stage();

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
		if (m_modifications_ready.load(std::memory_order_acquire))
		{
			std::swap(m_read_stage, m_swap_stage);

			m_modifications_ready.store(false, std::memory_order_release);
		}

		m_read_stage.merge();

		{
			std::lock_guard lock(m_command_mutex);
			m_command_read = std::move(m_command_write);
		}

		while (m_entity_pool.size() < k_entity_pool_max)
		{
			m_entity_pool.push_back(m_world.entity());
		}
	}

	void EntityLoader::ThreadLoop()
	{
		while (m_running.load(std::memory_order_acquire))
		{
			std::vector<Command> command_read;

			{
				std::lock_guard lock(m_command_mutex);
				command_read = std::move(m_command_read);
			}

			m_modifications_added = false;

			for (const Command& command : command_read)
			{
				switch (command.type)
				{
				case CommandType::LoadEntity:
					LoadEntity(command.uuid);
					break;
				}
			}

			if (m_modifications_added && !m_modifications_ready.load(std::memory_order_acquire))
			{
				std::swap(m_write_stage, m_swap_stage);

				m_modifications_ready.store(true, std::memory_order_release);
			}
		}
	}

	void EntityLoader::LoadEntity(UUID id)
	{

	}
}