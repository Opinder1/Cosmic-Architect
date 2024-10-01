#pragma once

#include "Util/UUID.h"
#include "Util/Time.h"
#include "Util/PerThread.h"
#include "Util/Nocopy.h"
#include "Util/Debug.h"

#include <godot_cpp/variant/string.hpp>

#include <flecs/flecs.h>

#include <robin_hood/robin_hood.h>

#include <TKRZW/tkrzw_dbm_shard.h>
#include <TKRZW/tkrzw_dbm_async.h>

#include <vector>
#include <deque>
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

			SaveAndUnloadEntity,
		};

		struct Command
		{
			CommandType type = CommandType::CreateEntity;

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

		struct LoadTask
		{
			UUID id;
			std::future<std::pair<tkrzw::Status, std::string>> future;
		};

	public:
		EntityLoader(flecs::world& world);
		~EntityLoader();

		void Progress();

	private:

		void ThreadLoop();

		void ProcessCommands();

		void CreateEntity(UUID id);
		void DeleteEntity(UUID id);
		void LoadEntity(UUID id);
		void UnloadEntity(UUID id);
		void ReloadEntity(UUID id);
		void SaveEntity(UUID id);
		void SaveAndUnloadEntity(UUID id);

		void ProcessCreateTasks();
		void ProcessDeleteTasks();
		void ProcessLoadTasks();
		void ProcessUnloadTasks();
		void ProcessReloadTasks();
		void ProcessSaveTasks();
		void ProcessSaveAndUnloadTasks();

	private:
		std::thread m_thread;
		std::atomic_bool m_running = false;
		flecs::world_t* m_world;

		// Commands requested to loader
		CommandSwapBuffer<Command> m_commands;

		// Modifications output by loader
		bool m_modifications_added = false; // Flag set when modifcations are made
		TripleBuffer<flecs::world> m_modification_stage;

		// Alive entity handles that can be used in the stages
		std::vector<flecs::entity_t> m_entity_pool; 

		// Cache of already loaded entities
		robin_hood::unordered_map<UUID, EntityData, UUIDHash> m_entity_cache;

		// Database that stores all entities on disk
		tkrzw::ShardDBM m_database;
		tkrzw::AsyncDBM m_database_async;

		std::deque<LoadTask> m_create_tasks;
		std::deque<LoadTask> m_delete_tasks;
		std::deque<LoadTask> m_load_tasks;
		std::deque<LoadTask> m_unload_tasks;
		std::deque<LoadTask> m_reload_tasks;
		std::deque<LoadTask> m_save_tasks;
		std::deque<LoadTask> m_save_and_unload_tasks;

#if DEBUG
		std::thread::id m_owner_id; // The thread that owns the loader and should call Progress() on it
#endif
	};
}