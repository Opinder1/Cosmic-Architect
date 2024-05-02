#pragma once

#include <godot_cpp/variant/string_name.hpp>

#include <vector>

namespace voxel_game
{
	struct SignalsComponent
	{
		std::vector<godot::StringName> events;
	};
}