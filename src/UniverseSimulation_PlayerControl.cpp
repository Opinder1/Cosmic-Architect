#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

namespace voxel_game
{
	void UniverseSimulation::SetVelocity(const godot::Vector3& velocity)
	{
		SIM_DEFER_COMMAND(k_commands->set_velocity, velocity);
	}

	void UniverseSimulation::Accelerate(const godot::Vector3& acceleration)
	{
		SIM_DEFER_COMMAND(k_commands->accelerate, acceleration);
	}

	void UniverseSimulation::SetLookDirection(const godot::Quaternion& direction)
	{
		SIM_DEFER_COMMAND(k_commands->set_look_direction, direction);
	}

	void UniverseSimulation::SetRotationVelocity(const godot::Quaternion& velocity)
	{
		SIM_DEFER_COMMAND(k_commands->set_rotation_velocity, velocity);
	}

	void UniverseSimulation::AccelerateRotation(const godot::Quaternion& acceleration)
	{
		SIM_DEFER_COMMAND(k_commands->accelerate_rotation, acceleration);
	}

	void UniverseSimulation::SetSprint(bool is_sprinting)
	{
		SIM_DEFER_COMMAND(k_commands->set_sprint, is_sprinting);
	}

	void UniverseSimulation::SetCrouching(bool is_crouching)
	{
		SIM_DEFER_COMMAND(k_commands->set_crouching, is_crouching);
	}

	void UniverseSimulation::SetProne(bool is_prone)
	{
		SIM_DEFER_COMMAND(k_commands->set_prone, is_prone);
	}

	void UniverseSimulation::Jump(double power)
	{
		SIM_DEFER_COMMAND(k_commands->jump, power);
	}
}