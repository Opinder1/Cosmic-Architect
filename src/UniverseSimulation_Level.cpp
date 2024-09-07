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
		SIM_DEFER_COMMAND(k_commands->complete_level_up, levelup_choices);
	}
}