#pragma once

#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/color.hpp>

#include <vector>

namespace voxel_game::galaxy
{
	// This entity is a galaxy
	struct CGalaxy
	{
		godot::String path;
	};

	// This entity is a star
	struct CStar
	{
		godot::Color color;
	};
}