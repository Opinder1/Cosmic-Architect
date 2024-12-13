#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

namespace voxel_game
{
	godot::Dictionary UniverseSimulation::GetAbilityInfo(const UUID& ability_id)
	{
		std::shared_lock lock(m_info_cache.mutex);

		auto it = m_info_cache.ability_info_map.find(ability_id);

		if (it != m_info_cache.ability_info_map.end())
		{
			return it->second;
		}
		else
		{
			return godot::Dictionary{};
		}
	}

	void UniverseSimulation::ActivateAbility(const UUID& ability_id)
	{
		if (DeferCommand<&UniverseSimulation::ActivateAbility>(ability_id))
		{
			return;
		}

	}

	void UniverseSimulation::ToggleAbility(const UUID& ability_id, bool toggled)
	{
		if (DeferCommand<&UniverseSimulation::ToggleAbility>(ability_id, toggled))
		{
			return;
		}
	}

	void UniverseSimulation::SetPlayerSetting(const UUID& setting_id, const godot::Variant& value)
	{
		if (DeferCommand<&UniverseSimulation::SetPlayerSetting>(setting_id, value))
		{
			return;
		}
	}
}