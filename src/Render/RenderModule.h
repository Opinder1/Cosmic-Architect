#pragma once

#include <godot_cpp/variant/rid.hpp>

#include <flecs/flecs.h>

namespace voxel_game::rendering
{
	struct Module
	{
		Module(flecs::world& world);
	};

	bool IsEnabled();
}