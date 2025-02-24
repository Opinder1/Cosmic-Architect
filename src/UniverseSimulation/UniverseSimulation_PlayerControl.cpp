#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

#include "Physics3D/PhysicsComponents.h"

namespace voxel_game
{

	void UniverseSimulation::SetSprint(bool is_sprinting)
	{
		if (DeferCommand<&UniverseSimulation::SetSprint>(is_sprinting))
		{
			return;
		}
	}

	void UniverseSimulation::SetCrouching(bool is_crouching)
	{
		if (DeferCommand<&UniverseSimulation::SetCrouching>(is_crouching))
		{
			return;
		}
	}

	void UniverseSimulation::SetProne(bool is_prone)
	{
		if (DeferCommand<&UniverseSimulation::SetProne>(is_prone))
		{
			return;
		}
	}

	void UniverseSimulation::DoWalk(const godot::Vector3& velocity)
	{
		if (DeferCommand<&UniverseSimulation::DoWalk>(velocity))
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

	void UniverseSimulation::DoLook(const godot::Quaternion& direction)
	{
		if (DeferCommand<&UniverseSimulation::DoLook>(direction))
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

	void UniverseSimulation::DoJump(double power)
	{
		if (DeferCommand<&UniverseSimulation::DoJump>(power))
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