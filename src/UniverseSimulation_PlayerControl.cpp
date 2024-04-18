#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

namespace voxel_game
{
	void UniverseSimulation::SetMoveForwards(bool is_moving)
	{
		SIM_DEFER_COMMAND(k_commands->set_move_forwards, is_moving);
	}

	void UniverseSimulation::SetMoveBackwards(bool is_moving)
	{
		SIM_DEFER_COMMAND(k_commands->set_move_backwards, is_moving);
	}

	void UniverseSimulation::SetMoveLeft(bool is_moving)
	{
		SIM_DEFER_COMMAND(k_commands->set_move_left, is_moving);
	}

	void UniverseSimulation::SetMoveRight(bool is_moving)
	{
		SIM_DEFER_COMMAND(k_commands->set_move_right, is_moving);
	}

	void UniverseSimulation::SetMoveUp(bool is_moving)
	{
		SIM_DEFER_COMMAND(k_commands->set_move_up, is_moving);
	}

	void UniverseSimulation::SetMoveDown(bool is_moving)
	{
		SIM_DEFER_COMMAND(k_commands->set_move_down, is_moving);
	}

	void UniverseSimulation::SetRotateLeft(bool is_rotating)
	{
		SIM_DEFER_COMMAND(k_commands->set_rotate_left, is_rotating);
	}

	void UniverseSimulation::SetRotateRight(bool is_rotating)
	{
		SIM_DEFER_COMMAND(k_commands->set_rotate_right, is_rotating);
	}

	void UniverseSimulation::SetLookDirection(const godot::Quaternion& direction)
	{
		SIM_DEFER_COMMAND(k_commands->set_look_direction, direction);
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