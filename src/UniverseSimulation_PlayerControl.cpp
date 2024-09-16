#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

#include "Physics3D/PhysicsComponents.h"

namespace voxel_game
{

	void UniverseSimulation::SetSprint(bool is_sprinting)
	{
		if (DeferCommand(k_commands->set_sprint, is_sprinting))
		{
			return;
		}
	}

	void UniverseSimulation::SetCrouching(bool is_crouching)
	{
		if (DeferCommand(k_commands->set_crouching, is_crouching))
		{
			return;
		}
	}

	void UniverseSimulation::SetProne(bool is_prone)
	{
		if (DeferCommand(k_commands->set_prone, is_prone))
		{
			return;
		}
	}

	void UniverseSimulation::DoWalk(const godot::Vector3& velocity)
	{
		if (DeferCommand(k_commands->do_walk, velocity))
		{
			return;
		}

		if (!m_player_entity)
		{
			return;
		}

		flecs::entity player_entity(m_world, m_player_entity);

		player_entity.get_mut<physics3d::Velocity>()->velocity = velocity;
		player_entity.modified<physics3d::Velocity>();
	}

	void UniverseSimulation::DoLook(const godot::Quaternion& direction)
	{
		if (DeferCommand(k_commands->do_look, direction))
		{
			return;
		}

		if (!m_player_entity)
		{
			return;
		}

		flecs::entity player_entity(m_world, m_player_entity);

		player_entity.get_mut<physics3d::Rotation>()->rotation = direction;
		player_entity.modified<physics3d::Rotation>();
	}

	void UniverseSimulation::DoJump(double power)
	{
		if (DeferCommand(k_commands->do_jump, power))
		{
			return;
		}

		if (!m_player_entity)
		{
			return;
		}

		m_world.query_builder<physics3d::Velocity, const physics3d::Gravity>()
			.term_at(0).src(m_player_entity)
			.term_at(1).up(flecs::ChildOf)
			.each([power](physics3d::Velocity& velocity, const physics3d::Gravity& gravity)
		{
			velocity.velocity = gravity.force * power * -1;
		});
	}
}