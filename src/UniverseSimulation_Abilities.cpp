#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

namespace voxel_game
{
	godot::Dictionary UniverseSimulation::GetAbilityInfo(UUID ability_id)
	{
		std::shared_lock lock(m_cache_mutex);
		return GetCacheEntry(ability_id);
	}

	void UniverseSimulation::ActivateAbility(UUID ability_id)
	{
		SIM_DEFER_COMMAND(k_commands->activate_ability, ability_id);

	}

	void UniverseSimulation::ToggleAbility(UUID ability_id, bool toggled)
	{
		SIM_DEFER_COMMAND(k_commands->toggle_ability, ability_id, toggled);
	}

	void UniverseSimulation::SetPlayerSetting(UUID setting_id, const godot::Variant& value)
	{
		SIM_DEFER_COMMAND(k_commands->set_player_setting, setting_id, value);
	}
}