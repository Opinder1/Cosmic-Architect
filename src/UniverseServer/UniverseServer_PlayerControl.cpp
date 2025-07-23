#include "UniverseServer.h"
#include "UniverseServer_StringNames.h"

#include "Components.h"

namespace voxel_game
{

	void UniverseServer::SetSprint(bool is_sprinting)
	{
		if (DeferCommand<&UniverseServer::SetSprint>(is_sprinting))
		{
			return;
		}
	}

	void UniverseServer::SetCrouching(bool is_crouching)
	{
		if (DeferCommand<&UniverseServer::SetCrouching>(is_crouching))
		{
			return;
		}
	}

	void UniverseServer::SetProne(bool is_prone)
	{
		if (DeferCommand<&UniverseServer::SetProne>(is_prone))
		{
			return;
		}
	}

	void UniverseServer::DoWalk(const godot::Vector3& velocity)
	{
		if (DeferCommand<&UniverseServer::DoWalk>(velocity))
		{
			return;
		}

		if (!m_player_entity)
		{
			return;
		}

		m_player_entity->*&CVelocity::velocity = velocity;
	}

	void UniverseServer::DoLook(const godot::Quaternion& direction)
	{
		if (DeferCommand<&UniverseServer::DoLook>(direction))
		{
			return;
		}

		if (!m_player_entity)
		{
			return;
		}

		m_player_entity->*&CRotation::rotation = direction;
	}

	void UniverseServer::DoJump(double power)
	{
		if (DeferCommand<&UniverseServer::DoJump>(power))
		{
			return;
		}

		if (!m_player_entity)
		{
			return;
		}

		m_player_entity->*&CVelocity::velocity = m_player_entity->*&CGravity::force * power * -1;
	}
}