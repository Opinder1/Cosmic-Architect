#include "UniverseServer.h"
#include "UniverseServer_StringNames.h"

namespace voxel_game
{
	void UniverseServer::TriggerVehicleControl(const UUID& control_id)
	{
		if (DeferCommand<&UniverseServer::TriggerVehicleControl>(control_id))
		{
			return;
		}
	}

	void UniverseServer::ToggleVehicleControl(const UUID& control_id, bool toggled)
	{
		if (DeferCommand<&UniverseServer::ToggleVehicleControl>(control_id, toggled))
		{
			return;
		}
	}

	void UniverseServer::SetVehicleControl(const UUID& control_id, const godot::Variant& value)
	{
		if (DeferCommand<&UniverseServer::SetVehicleControl>(control_id, value))
		{
			return;
		}
	}
}