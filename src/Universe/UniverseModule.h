#pragma once

#include <flecs/flecs.h>

namespace godot
{
	class StringName;
	class RID;
}

namespace voxel_game::universe
{
	struct Module
	{
		Module(flecs::world& world);
	};

	flecs::entity CreateNewUniverse(flecs::world& world, const godot::StringName& path);
}