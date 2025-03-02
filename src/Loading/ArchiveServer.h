#pragma once

#include "Commands/TypedCommandBuffer.h"

#include "Util/Util.h"
#include "Util/PerThread.h"
#include "Util/Debug.h"
#include "Util/Hash.h"

#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/string_name.hpp>
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
	const size_t k_num_database_workers = 2;

	using EntitySaveData = std::vector<char>;

	// An asynchronous entity loader that manages generating and loading entities from disk. Entities can have dependenices on other entities
	// that will be loaded or generated as needed. The entities added by this loader should only be removed by this loader.
	class ArchiveServer : Nomove, Nocopy
	{
	public:
		using DBHandle = size_t;

		enum ThreadMode
		{
			SingleThreaded,
			MultiThreaded
		};

	private:
		// Cached info about an entity that is loaded and unloaded
		struct EntityData
		{
			flecs::entity_t entity = 0;
			DBHandle database = 0;

			size_t refcount = 0;

			std::vector<UUID> dependencies;

			Clock::time_point load_time;
			Clock::time_point last_reference_time;
		};

		// Database that stores all entities on disk
		struct Database
		{
			Database() :
				database_async(&database, k_num_database_workers)
			{}

			godot::String path;

			tkrzw::ShardDBM database;
			tkrzw::AsyncDBM database_async;

			bool closing = false;

			std::vector<EntityData*> entities;
		};

		struct LoadTask
		{
			UUID uuid;
			EntityData* entity = nullptr;
		};

		struct SaveTask
		{
			UUID uuid;
			EntityData* entity = nullptr;
			EntitySaveData data;
		};

		struct DeleteTask
		{
			UUID uuid;
			EntityData* entity = nullptr;
		};
		
		struct Worker
		{
			flecs::world_t* world = nullptr;

			bool modifications_added = false; // Flag set when modifcations are made

			std::deque<LoadTask> load_tasks;
			std::deque<SaveTask> save_tasks;
			std::deque<DeleteTask> delete_tasks;

			// Alive entity handles that can be used in the stages
			std::vector<flecs::entity_t> entity_pool;

			// Cache of already loaded entities
			robin_hood::unordered_node_map<UUID, EntityData, UUIDHash> entity_cache;

			robin_hood::unordered_node_map<DBHandle, Database> databases;
		};

	public:
		ArchiveServer();
		~ArchiveServer();

		void Initialize(flecs::world& world, ThreadMode thread_mode);

		bool IsThreaded();

		void Progress();

		DBHandle OpenArchive(const godot::StringName& path);
		void CloseArchive(const godot::StringName& path);
		bool IsArchiveOpen(const godot::StringName& path);

		void LoadEntity(UUID uuid, flecs::entity_t entity, DBHandle db_handle);
		void SaveEntity(UUID uuid, flecs::entity_t entity, DBHandle db_handle, EntitySaveData&& data);
		void DeleteEntity(UUID uuid, flecs::entity_t entity, DBHandle db_handle);

	private:
		void ThreadLoop();

		bool HasTasks();

		void DoOpenCommand(godot::StringName path, DBHandle db_handle);
		void DoCloseCommand(DBHandle db_handle);
		void DoLoadCommand(UUID uuid, flecs::entity_t entity, DBHandle db_handle);
		void DoSaveCommand(UUID uuid, flecs::entity_t entity, DBHandle db_handle, EntitySaveData&& data);
		void DoDeleteCommand(UUID uuid, flecs::entity_t entity, DBHandle db_handle);
		void ProcessCommands();

		bool ProcessLoadTask(LoadTask& task);
		bool ProcessSaveTask(SaveTask& task);
		bool ProcessDeleteTask(DeleteTask& task);
		void ProcessTasks();

	private:
		std::thread m_thread;
		std::atomic_bool m_running = false;

		// Commands requested to loader by owner thread
		alignas(k_cache_line) TCommandBuffer<ArchiveServer> m_commands_write;
		alignas(k_cache_line) SwapBuffer<TCommandBuffer<ArchiveServer>> m_commands_swap;

		// Modifications output by loader to owner thread
		alignas(k_cache_line) flecs::world m_modifications_write;
		alignas(k_cache_line) flecs::world m_modifications_read;
		alignas(k_cache_line) SwapBuffer<flecs::world> m_modifications_swap;

		// List of open databases for use by. Not thread safe
		robin_hood::unordered_flat_map<godot::StringName, DBHandle> m_open_databases;

		// Counter for generating new database handles. Not thread safe
		DBHandle m_new_handle = 0;

		// Worker thread only members
		Worker m_worker;

#if defined(DEBUG_ENABLED)
		std::thread::id m_owner_id; // The thread that owns the loader and calls Process() on it
#endif
	};
}