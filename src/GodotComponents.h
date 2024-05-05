#pragma once

#include <godot_cpp/variant/string_name.hpp>

#include <vector>

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

	struct GodotComponents
	{
		GodotComponents(flecs::world& world);
	};
}