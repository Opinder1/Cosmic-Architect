#pragma once

#include "ArchiveServer.h"

#include "Util/GodotUUID.h"

#include <godot_cpp/variant/string_name.hpp>

#include <flecs/flecs.h>

namespace voxel_game::loading
{
	struct CIdentifier
	{
		UUID uuid;
	};

	struct CSaveable
	{
		uint64_t last_load_frame = 0;
		uint64_t last_save_frame = 0;
	};

	struct CDatabase
	{
		godot::StringName path;
		ArchiveServer::DBHandle handle;
	};

	struct Components
	{
		Components(flecs::world& world);
	};
}