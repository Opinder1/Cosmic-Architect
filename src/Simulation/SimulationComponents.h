#pragma once

#include "Util/UUID.h"
#include "Util/Util.h"
#include "Util/PerThread.h"
#include "Util/SmallVector.h"
#include "Util/Nocopy.h"

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/dir_access.hpp>

#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/dictionary.hpp>

#include <robin_hood/robin_hood.h>

#include <vector>

namespace voxel_game::simulation
{
	constexpr const size_t k_max_pool_entities = 1024;

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
		godot::StringName path;
		godot::Dictionary values;
	};

	struct CThreadWorker
	{
		size_t index;
	};

	struct CUUID
	{
		UUID id;
	};
}