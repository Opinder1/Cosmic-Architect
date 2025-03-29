#pragma once

#include "GalaxyWorld.h"

#include <godot_cpp/variant/color.hpp>

#include <vector>

namespace voxel_game::galaxy
{
	// This entity is a galaxy
	struct CWorld {};

	// This entity is a star
	struct CStar
	{
		godot::Color color;
	};
}