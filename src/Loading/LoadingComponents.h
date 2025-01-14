#pragma once

#include "EntityLoader.h"

#include "Util/UUID.h"

#include <godot_cpp/variant/string_name.hpp>

namespace flecs
{
	class world;
}

namespace voxel_game::loading
{
	struct SaveEvent
	{
		std::string data;
	};

	struct Identifier
	{
		UUID uuid;
	};

	struct Saveable
	{
		uint64_t last_load_frame = 0;
		uint64_t last_save_frame = 0;
	};

	struct AutoLoad {};

	struct AutoSave
	{
		uint64_t frequency = 0;
	};

	struct Database
	{
		godot::StringName path;
		EntityLoader::DBHandle handle;
	};

	struct Components
	{
		Components(flecs::world& world);
	};
}