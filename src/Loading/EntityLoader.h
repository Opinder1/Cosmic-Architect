#pragma once

#include "Util/UUID.h"
#include "Util/Time.h"
#include "Util/PerThread.h"
#include "Util/Nocopy.h"
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

	using EntitySaveData = std::string;

	// An asynchronous entity loader that manages generating and loading entities from disk. Entities can have dependenices on other entities
	// that will be loaded or generated as needed. The entities added by this loader should only be removed by this loader.
	class EntityLoader : Nomove, Nocopy
	{
	public:
		using DBHandle = size_t;

		using ComponentRead = void(const std::string_view& buffer);
		using ComponentWrite = void(std::vector<char>& buffer);

	private:
		// Cached info about an entity that is loaded and unloaded
		struct Entity
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

			std::vector<Entity*> entities;
		};

		struct OpenCommand
		{
			godot::StringName path;
			DBHandle handle;
		};

		struct CloseCommand
		{
			DBHandle handle;
		};

		struct LoadCommand
		{
			UUID uuid;
			flecs::entity_t entity = 0;
			DBHandle database;
		};

		struct SaveCommand
		{
			UUID uuid;
			flecs::entity_t entity = 0;
			DBHandle database;
			EntitySaveData data;
		};

		struct DeleteCommand
		{
			UUID uuid;
			flecs::entity_t entity = 0;
			DBHandle database;
		};

		struct LoadTask
		{
			UUID uuid;
			Entity* entity = 0;
		};

	public:
		EntityLoader();
		~EntityLoader();

		void Initialize(flecs::world& world);

		void Progress();

		DBHandle OpenArchive(const godot::StringName& path);
		void CloseArchive(const godot::StringName& path);
		bool IsArchiveOpen(const godot::StringName& path);

		void AddComponent(uint16_t id, flecs::entity_t entity, ComponentRead read, ComponentWrite write);

		void LoadEntity(UUID uuid, flecs::entity_t entity, DBHandle database);
		void SaveEntity(UUID uuid, flecs::entity_t entity, DBHandle database, EntitySaveData&& data);
		void DeleteEntity(UUID uuid, flecs::entity_t entity, DBHandle database);

	private:
		void ThreadLoop();

		bool HasTasks();

		void DoOpenCommand(OpenCommand& command);
		void DoCloseCommand(CloseCommand& command);
		void DoLoadCommand(LoadCommand& command);
		void DoSaveCommand(SaveCommand& command);
		void DoDeleteCommand(DeleteCommand& command);
		void ProcessCommands();

		bool ProcessLoadTask(Entity& entity);
		bool ProcessSaveTask(Entity& entity);
		bool ProcessDeleteTask(Entity& entity);
		void ProcessTasks();

	private:
		std::thread m_thread;
		std::atomic_bool m_running = false;

		// Commands requested to loader by owner thread
		CommandSwapBuffer<OpenCommand> m_open_commands;
		CommandSwapBuffer<CloseCommand> m_close_commands;
		CommandSwapBuffer<LoadCommand> m_load_commands;
		CommandSwapBuffer<SaveCommand> m_save_commands;
		CommandSwapBuffer<DeleteCommand> m_delete_commands;

		// Modifications output by loader to owner thread
		TripleBuffer<flecs::world> m_modification_stage;

		// List of open databases for use by 
		robin_hood::unordered_flat_map<godot::StringName, DBHandle> m_open_databases;

		// Counter for generating new database handles
		DBHandle m_new_handle = 0;

		struct
		{
			flecs::world_t* world = nullptr;

			bool modifications_added = false; // Flag set when modifcations are made

			// Alive entity handles that can be used in the stages
			std::vector<flecs::entity_t> entity_pool;

			// Cache of already loaded entities
			robin_hood::unordered_node_map<UUID, Entity, UUIDHash> entity_cache;

			std::deque<LoadTask> load_tasks;
			std::deque<Entity*> save_tasks;
			std::deque<Entity*> delete_tasks;

			robin_hood::unordered_node_map<DBHandle, Database> databases;
		} m_worker;
	};
}