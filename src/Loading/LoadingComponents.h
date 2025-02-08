#pragma once

#include "ArchiveServer.h"

#include "Util/UUID.h"

#include <godot_cpp/variant/string_name.hpp>

#include <flecs/flecs.h>

namespace voxel_game::loading
{
	struct Identifier
	{
		UUID uuid;
	};

	struct Saveable
	{
		uint64_t last_load_frame = 0;
		uint64_t last_save_frame = 0;
	};

	struct Database
	{
		godot::StringName path;
		ArchiveServer::DBHandle handle;
	};

	struct Components
	{
		Components(flecs::world& world);
	};
}