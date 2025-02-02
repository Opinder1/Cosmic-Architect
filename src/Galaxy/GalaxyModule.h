#pragma once

#include <flecs/flecs.h>

namespace godot
{
	class String;
}

namespace voxel_game::galaxy
{
	struct Module
	{
		Module(flecs::world& world);
	};

	flecs::entity CreateNewSimulatedGalaxy(flecs::world& world, const godot::String& path, flecs::entity_t universe_entity);
}