#pragma once

#include <TKRZW/tkrzw_dbm_shard.h>
#include <TKRZW/tkrzw_dbm_async.h>

#include <optional>
#include <cstdint>

namespace flecs
{
	class world;
}

namespace voxel_game::loading
{
	enum class LoadState
	{
		Loading,
		Loaded,
		Unloading,
		Unloaded
	};

	struct Loadable
	{
		bool require_children = false; // Require any children added before starting loading to also load
		size_t children = 0; // Number of children we are waiting on to load
		size_t tasks = 0; // Number of tasks that systems have specified we need to do to load
	};

	struct LoadingPhase {};

	struct EntityLoader
	{
		EntityLoader(size_t num_workers) :
			async_db(&entity_db, num_workers)
		{}

		tkrzw::ShardDBM entity_db;
		tkrzw::AsyncDBM async_db;
	};

	struct Components
	{
		Components(flecs::world& world);
	};
}