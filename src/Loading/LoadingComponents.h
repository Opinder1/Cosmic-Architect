#pragma once

#include "ArchiveServer.h"

#include "Util/UUID.h"

#include <godot_cpp/variant/string_name.hpp>

namespace voxel_game::loading
{
	enum class State
	{
		Unloaded,
		Loading,
		Loaded,
		Unloading
	};

	struct CStreamable
	{
		State state = State::Unloaded;
		size_t tasks = 0;
	};

	struct CAutosave
	{
		uint64_t last_save_frame = 0;
	};
}