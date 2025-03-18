#pragma once

#include <godot_cpp/variant/string.hpp>

#include <flecs/flecs.h>

namespace voxel_game::player
{
	struct Module
	{
		Module(flecs::world& world);
	};

	flecs::entity_t CreateLocalPlayer(flecs::world& world, flecs::entity_t server, const godot::String& name);

	flecs::entity_t CreatePlayerAvatar(flecs::world& world, flecs::entity_t player);
}