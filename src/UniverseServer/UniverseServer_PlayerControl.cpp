#include "UniverseServer.h"
#include "UniverseServer_StringNames.h"

#include "Physics3D/PhysicsComponents.h"

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

		flecs::entity player_entity(m_world, m_player_entity);

		player_entity.get_mut<physics3d::CVelocity>()->velocity = velocity;
		player_entity.modified<physics3d::CVelocity>();
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

		flecs::entity player_entity(m_world, m_player_entity);

		player_entity.get_mut<physics3d::CRotation>()->rotation = direction;
		player_entity.modified<physics3d::CRotation>();
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

		m_world.query_builder<physics3d::CVelocity, const physics3d::CGravity>()
			.term_at(0).src(m_player_entity)
			.term_at(1).up(flecs::ChildOf)
			.each([power](physics3d::CVelocity& velocity, const physics3d::CGravity& gravity)
		{
			velocity.velocity = gravity.force * power * -1;
		});
	}
}