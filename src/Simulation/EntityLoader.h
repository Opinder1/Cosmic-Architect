#pragma once

#include "Util/UUID.h"
#include "Util/PerThread.h"

#include <robin_hood/robin_hood.h>

#include <flecs/flecs.h>

#include <TKRZW/tkrzw_thread_util.h>

#include <thread>

namespace voxel_game
{
	class EntityLoader
	{
	private:
		enum class CommandType
		{
			LoadEntity,
			SaveEntity,
			InstanceEntity,
			UnloadEntity,
		};

		struct Command
		{
			CommandType type;

			union // Command data
			{
				flecs::entity_t entity;
				UUID uuid;
			};
		};

	public:
		EntityLoader(const flecs::world& world);
		~EntityLoader();

		void Progress();

	private:
		void ThreadLoop();

		void LoadEntity(UUID id);

	private:
		std::thread m_thread;
		std::atomic_bool m_running = false;

		// Commands requested to loader
		tkrzw::SpinMutex m_command_mutex;
		alignas(k_cache_line) std::vector<Command> m_command_write;
		alignas(k_cache_line) std::vector<Command> m_command_read;

		// Modifications output by loader
		struct alignas(k_cache_line)
		{
			std::atomic_bool m_modifications_ready = false;
			flecs::world m_write_stage;
			flecs::world m_swap_stage;
			flecs::world m_read_stage;
		};

		// Flag set when modifcations are made
		bool m_modifications_added = false;

		// Alive entity handles that can be used in the stages and world reference to generate more
		flecs::world m_world;
		std::vector<flecs::entity_t> m_entity_pool;

		// Cache of already loaded entities
		robin_hood::unordered_map<UUID, flecs::entity_t, UUIDHash> m_entity_cache;
	};
}