#pragma once

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/dir_access.hpp>

#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/string_name.hpp>

namespace flecs
{
	struct world;
}

namespace voxel_game
{
	struct SignalsComponent
	{
		std::vector<godot::StringName> events;
	};

	struct UniverseComponent {};

	struct UniverseObjectComponent {};

	struct UniverseComponents
	{
		UniverseComponents(flecs::world& world);
	};
}