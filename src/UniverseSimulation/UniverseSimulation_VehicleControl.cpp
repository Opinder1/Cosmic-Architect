#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

namespace voxel_game
{
	void UniverseSimulation::TriggerVehicleControl(const UUID& control_id)
	{
		if (DeferCommand<&UniverseSimulation::TriggerVehicleControl>(control_id))
		{
			return;
		}
	}

	void UniverseSimulation::ToggleVehicleControl(const UUID& control_id, bool toggled)
	{
		if (DeferCommand<&UniverseSimulation::ToggleVehicleControl>(control_id, toggled))
		{
			return;
		}
	}

	void UniverseSimulation::SetVehicleControl(const UUID& control_id, const godot::Variant& value)
	{
		if (DeferCommand<&UniverseSimulation::SetVehicleControl>(control_id, value))
		{
			return;
		}
	}
}