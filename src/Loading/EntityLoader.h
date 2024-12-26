#pragma once

#include "Util/UUID.h"
#include "Util/Time.h"
#include "Util/PerThread.h"
#include "Util/Nocopy.h"
#include "Util/Debug.h"

#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/variant.hpp>

#include <flecs/flecs.h>

#include <robin_hood/robin_hood.h>

#include <TKRZW/tkrzw_dbm_shard.h>
#include <TKRZW/tkrzw_dbm_async.h>

#include <vector>
#include <deque>
#include <thread>
#include <memory>

namespace voxel_game::loading
{
	using EntitySaveData = robin_hood::unordered_flat_map<std::string, godot::Variant>;

	// An asynchronous entity loader that manages generating and loading entities from disk. Entities can have dependenices on other entities
	// that will be loaded or generated as needed. The entities added by this loader should only be removed by this loader.
	class EntityLoader : Nomove
	{
	private:
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
			flecs::entity_t entity;
			UUID uuid;
			std::future<std::pair<tkrzw::Status, std::string>> future;
		};

		struct SaveTask
		{
			flecs::entity_t entity;
			EntitySaveData data;
		};

		struct DeleteTask
		{
			flecs::entity_t entity;
		};

	public:
		EntityLoader();
		~EntityLoader();

#if defined(DEBUG_ENABLED)
		void SetProgressThread(std::thread::id thread_id);
#endif

		void Initialize(flecs::world& world);

		void Progress();

		void LoadEntity(flecs::entity_t entity, UUID uuid);
		void SaveEntity(flecs::entity_t entity, EntitySaveData&& data);
		void DeleteEntity(flecs::entity_t entity);

	private:
		void ThreadLoop();

		void ProcessLoadTasks();
		void ProcessSaveTasks();
		void ProcessDeleteTasks();

	private:
		std::thread m_thread;
		std::atomic_bool m_running = false;
		flecs::world_t* m_world = nullptr;

		// Commands requested to loader
		CommandSwapBuffer<LoadTask> m_load_commands;
		CommandSwapBuffer<SaveTask> m_save_commands;
		CommandSwapBuffer<DeleteTask> m_delete_commands;

		// Modifications output by loader
		bool m_modifications_added = false; // Flag set when modifcations are made
		TripleBuffer<flecs::world> m_modification_stage;

		// Alive entity handles that can be used in the stages
		std::vector<flecs::entity_t> m_entity_pool; 

		// Cache of already loaded entities
		robin_hood::unordered_map<flecs::entity_t, UUID> m_entity_to_uuid;
		robin_hood::unordered_map<UUID, EntityData, UUIDHash> m_entity_cache;

		// Database that stores all entities on disk
		tkrzw::ShardDBM m_database;
		tkrzw::AsyncDBM m_database_async;

		std::deque<LoadTask> m_load_tasks;
		std::deque<SaveTask> m_save_tasks;
		std::deque<DeleteTask> m_delete_tasks;

#if defined(DEBUG_ENABLED)
		std::thread::id m_owner_id; // The thread that owns the loader and should call Progress() on it
#endif
	};
}