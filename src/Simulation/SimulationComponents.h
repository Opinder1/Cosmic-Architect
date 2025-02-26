#pragma once

#include "Util/Util.h"
#include "Util/PerThread.h"
#include "Util/SmallVector.h"
#include "Util/Nocopy.h"

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/dir_access.hpp>

#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/dictionary.hpp>

#include <flecs/flecs.h>

#include <vector>

namespace voxel_game::sim
{
	constexpr const size_t k_max_pool_entities = 1024;

	struct Components
	{
		Components(flecs::world& world);
	};

	struct CFrame
	{
		uint64_t frame_index = 0;
		Clock::time_point frame_start_time;
	};

	struct CLocalTime
	{
		Clock::time_point epoch;
	};

	struct CPath
	{
		godot::StringName path;
	};

	struct CConfig
	{
		godot::Dictionary values;
	};

	struct CThreadWorker
	{
		size_t index;
	};

	class ThreadEntityPool
	{
	public:
		ThreadEntityPool();

		void SetStage(flecs::world_t* stage);

		void AllocateEntities(flecs::world_t* world);

		void ClearEntities(flecs::world_t* world);

		flecs::entity CreateEntity();

	private:
		flecs::world_t* m_stage = nullptr;
		SmallVector<flecs::entity_t, k_max_pool_entities> m_new_entities;
	};

	struct CEntityPools : Nocopy
	{
		PerThread<ThreadEntityPool> threads;
	};
}