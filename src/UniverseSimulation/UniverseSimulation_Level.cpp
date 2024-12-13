#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

namespace voxel_game
{
	uint16_t UniverseSimulation::GetLevel()
	{
		std::shared_lock lock(m_info_cache.mutex);
		return m_info_cache.player_info.find_key("level");
	}

	uint64_t UniverseSimulation::GetExperience()
	{
		std::shared_lock lock(m_info_cache.mutex);
		return m_info_cache.player_info.find_key("experience");
	}

	void UniverseSimulation::CompleteLevelUp(const godot::Dictionary& levelup_choices)
	{
		if (DeferCommand<&UniverseSimulation::CompleteLevelUp>(levelup_choices))
		{
			return;
		}
	}
}