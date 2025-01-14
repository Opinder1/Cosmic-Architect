#pragma once

#include "Util/PerThread.h"
#include "Util/SmallVector.h"
#include "Util/Time.h"
#include "Util/Nocopy.h"

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/dir_access.hpp>

#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/dictionary.hpp>

#include <flecs/flecs.h>

#include <vector>

namespace flecs
{
	struct world;
}

namespace voxel_game::sim
{
	constexpr const size_t k_max_pool_entities = 1024;

	struct Components
	{
		Components(flecs::world& world);
	};

	struct GlobalTime
	{
		uint64_t frame_index = 0;
		Clock::time_point frame_start;
	};

	struct LocalTime
	{
		Clock::time_point epoch;
	};

	struct Config
	{
		godot::Dictionary values;
	};

	class ThreadEntityPool
	{
	public:
		ThreadEntityPool();

		void AllocateEntities(flecs::world_t* world);

		void ClearEntities(flecs::world_t* world);

		flecs::entity_t CreateThreadEntity();

	private:
		SmallVector<flecs::entity_t, k_max_pool_entities> m_new_entities;
	};

	struct ThreadEntityPools : Nocopy
	{
		PerThread<ThreadEntityPool> threads;
	};

	ThreadEntityPool& GetThreadEntityPool(ThreadEntityPools& pools, flecs::world_t* stage);
}