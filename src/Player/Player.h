#pragma once

#include "Entity/EntityPoly.h"

#include "Util/UUID.h"

#include <godot_cpp/variant/string.hpp>

#include <vector>

namespace voxel_game::player
{
	// Tree to find players avatar
	struct AvatarPath
	{
		std::vector<UUID> storage_tree; // Entities to traverse to find avatar
		UUID id; // Id of entity in storage
		entity::Ref cached_entity; // Cached entity if we have already found it
	};

	struct Player
	{
		godot::String username;
		godot::String ip;

		std::vector<std::unique_ptr<AvatarPath>> avatars;

		AvatarPath* current_avatar;

		std::vector<UUID> friends;
	};
}