#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

namespace voxel_game
{
	void UniverseSimulation::TriggerVehicleControl(const UUID& control_id)
	{
		if (DeferCommand(k_commands->trigger_vehicle_control, control_id))
		{
			return;
		}
	}

	void UniverseSimulation::ToggleVehicleControl(const UUID& control_id, bool toggled)
	{
		if (DeferCommand(k_commands->toggle_vehicle_control, control_id, toggled))
		{
			return;
		}
	}

	void UniverseSimulation::SetVehicleControl(const UUID& control_id, const godot::Variant& value)
	{
		if (DeferCommand(k_commands->set_vehicle_control, control_id, value))
		{
			return;
		}
	}
}