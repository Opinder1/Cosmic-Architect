#pragma once

#include "Simulation/Config.h"

#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/color.hpp>

#include <vector>

namespace voxel_game::galaxy
{
	// This entity is a galaxy
	struct CGalaxy
	{

	};

	// This entity is a star
	struct CStar
	{
		godot::Color color;
	};
}