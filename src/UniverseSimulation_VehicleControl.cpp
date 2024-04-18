#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

namespace voxel_game
{
	void UniverseSimulation::Accelerate(bool is_accelerating)
	{
		SIM_DEFER_COMMAND(k_commands->accelerate, is_accelerating);
	}

	void UniverseSimulation::Deccelerate(bool is_decelerating)
	{
		SIM_DEFER_COMMAND(k_commands->decelerate, is_decelerating);
	}

	void UniverseSimulation::ActivateVehicleControl(UUID system_id)
	{
		SIM_DEFER_COMMAND(k_commands->activate_vehicle_control, system_id);
	}

	void UniverseSimulation::ToggleVehicleControl(UUID system_id, bool toggled)
	{
		SIM_DEFER_COMMAND(k_commands->toggle_vehicle_control, system_id, toggled);
	}

	void UniverseSimulation::SetVehicleSetting(UUID setting_id, const godot::Variant& value)
	{
		SIM_DEFER_COMMAND(k_commands->set_vehicle_setting, setting_id, value);
	}
}