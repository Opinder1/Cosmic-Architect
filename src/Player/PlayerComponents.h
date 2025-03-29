#pragma once

#include "Entity/EntityPoly.h"

#include "Util/UUID.h"

#include <godot_cpp/variant/string.hpp>

#include <vector>

namespace voxel_game::player
{
	struct EntityPtr
	{
		UUID storage; // Storage that entity is in
		UUID id; // Id of entity in storage
		entity::Ptr cached_entity; // Cached entity if we have already found it
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

	struct CPlayers
	{
		std::vector<UUID> players;
	};

	struct CPlayerAvatar
	{

	};
}