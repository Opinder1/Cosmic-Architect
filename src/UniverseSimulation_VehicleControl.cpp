#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

namespace voxel_game
{
	void UniverseSimulation::TriggerVehicleControl(UUID control_id)
	{
		SIM_DEFER_COMMAND(k_commands->trigger_vehicle_control, control_id);
	}

	void UniverseSimulation::ToggleVehicleControl(UUID control_id, bool toggled)
	{
		SIM_DEFER_COMMAND(k_commands->toggle_vehicle_control, control_id, toggled);
	}

	void UniverseSimulation::SetVehicleControl(UUID control_id, const godot::Variant& value)
	{
		SIM_DEFER_COMMAND(k_commands->set_vehicle_control, control_id, value);
	}
}