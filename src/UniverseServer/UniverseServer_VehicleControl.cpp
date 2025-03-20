#include "UniverseServer.h"
#include "UniverseServer_StringNames.h"

namespace voxel_game
{
	void UniverseServer::TriggerVehicleControl(const ID& control_id)
	{
		if (DeferCommand<&UniverseServer::TriggerVehicleControl>(control_id))
		{
			return;
		}
	}

	void UniverseServer::ToggleVehicleControl(const ID& control_id, bool toggled)
	{
		if (DeferCommand<&UniverseServer::ToggleVehicleControl>(control_id, toggled))
		{
			return;
		}
	}

	void UniverseServer::SetVehicleControl(const ID& control_id, const godot::Variant& value)
	{
		if (DeferCommand<&UniverseServer::SetVehicleControl>(control_id, value))
		{
			return;
		}
	}
}