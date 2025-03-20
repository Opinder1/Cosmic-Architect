#pragma once

#include "Util/UUID.h"

#include <godot_cpp/variant/string.hpp>

#include <flecs/flecs.h>

#include <vector>

namespace voxel_game::player
{
	struct EntityPtr
	{
		UUID storage; // Storage that entity is in
		UUID id; // Id of entity in storage
		flecs::entity_t cached_entity; // Cached entity if we have already found it
	};

	// Tree to find players avatar
	struct AvatarPath
	{
		std::vector<EntityPtr> tree; // Entities to traverse to find avatar
		EntityPtr avatar;
	};

	struct CPlayer
	{
		godot::String username;
		godot::String ip;

		std::vector<AvatarPath> avatars;

		UUID current_avatar;

		std::vector<UUID> friends;
	};

	struct GPlayers
	{
		std::vector<UUID> players;
	};

	struct CPlayerAvatar
	{

	};

	struct Components
	{
		Components(flecs::world& world);
	};
}