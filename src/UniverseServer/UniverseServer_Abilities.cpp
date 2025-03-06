#include "UniverseServer.h"
#include "UniverseServer_StringNames.h"

namespace voxel_game
{
	godot::Dictionary UniverseServer::GetAbilityInfo(const UUID& ability_id)
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

	void UniverseServer::ActivateAbility(const UUID& ability_id)
	{
		if (DeferCommand<&UniverseServer::ActivateAbility>(ability_id))
		{
			return;
		}

	}

	void UniverseServer::ToggleAbility(const UUID& ability_id, bool toggled)
	{
		if (DeferCommand<&UniverseServer::ToggleAbility>(ability_id, toggled))
		{
			return;
		}
	}

	void UniverseServer::SetPlayerSetting(const UUID& setting_id, const godot::Variant& value)
	{
		if (DeferCommand<&UniverseServer::SetPlayerSetting>(setting_id, value))
		{
			return;
		}
	}
}