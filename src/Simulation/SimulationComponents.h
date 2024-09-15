#pragma once

#include "Util/PerThread.h"
#include "Util/SmallVector.h"
#include "Util/Time.h"
#include "Util/Nocopy.h"

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/dir_access.hpp>

#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/string_name.hpp>

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

	struct ThreadEntityPool
	{
		SmallVector<flecs::entity_t, k_max_pool_entities> new_entities;
	};

	struct ThreadEntityPools : Nocopy
	{
		PerThread<ThreadEntityPool> threads;
	};

	struct Simulation : Nocopy
	{
		godot::StringName name;
		godot::StringName path;
		godot::StringName fragment_type;

		bool networked = false;
		bool is_remote = false;
		godot::StringName remote_ip;

		size_t main_seed = 0;

		godot::Ref<godot::DirAccess> m_directory;
	};

	ThreadEntityPool& GetThreadEntityPool(flecs::world_t* stage);

	flecs::entity_t CreateThreadEntity(ThreadEntityPool& entity_pool);

	flecs::entity_t CreateThreadEntity(flecs::world_t* stage);
}