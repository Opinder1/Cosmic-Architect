#pragma once

#include "Util/UUID.h"
#include "Util/Time.h"
#include "Util/PerThread.h"
#include "Util/Debug.h"

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
			CreateEntity,
			DeleteEntity,
			LoadEntity,
			UnloadEntity,
			ReloadEntity,
			SaveEntity,
			InstanceEntity,
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

		struct EntityData
		{
			flecs::entity_t entity;

			size_t refcount;

			std::vector<UUID> dependencies;

			Clock::time_point load_time;
			Clock::time_point last_reference_time;
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
		CommandSwapBuffer<Command> m_commands;

		flecs::world m_world; // World reference to generate more entities for the pool
		std::vector<flecs::entity_t> m_entity_pool; // Alive entity handles that can be used in the stages

		bool m_modifications_added = false; // Flag set when modifcations are made
		TripleBuffer<flecs::world> m_modification_stage; // Modifications output by loader

		// Cache of already loaded entities
		robin_hood::unordered_map<UUID, EntityData, UUIDHash> m_entity_cache;

#if DEBUG
		std::thread::id m_owner_id; // The thread that owns the loader and should call Progress() on it
#endif
	};
}