#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

#include "Physics/PhysicsComponents.h"

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

		player_entity.get_mut<Velocity3DComponent>()->velocity = velocity;
		player_entity.modified<Velocity3DComponent>();
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

		player_entity.get_mut<Rotation3DComponent>()->rotation = direction;
		player_entity.modified<Rotation3DComponent>();
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

		m_world.query_builder<Velocity3DComponent, const Gravity3DComponent>()
			.term_at(0).src(m_player_entity)
			.term_at(1).up(flecs::ChildOf)
			.each([power](Velocity3DComponent& velocity, const Gravity3DComponent& gravity)
		{
			velocity.velocity = gravity.force * power * -1;
		});
	}
}