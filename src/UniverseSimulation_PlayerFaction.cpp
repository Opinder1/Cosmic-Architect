#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

namespace voxel_game
{
	UUID UniverseSimulation::GetGlobalPlayerFaction()
	{
		std::shared_lock lock(m_cache_mutex);
		return m_info_cache.galaxy_info.find_key("player_faction");
	}

	UUID UniverseSimulation::GetPlayerFaction()
	{
		std::shared_lock lock(m_cache_mutex);
		return m_info_cache.player_info.find_key("faction");
	}

	void UniverseSimulation::RequestJoinPlayerFaction(UUID faction_id, const godot::String& message)
	{
		SIM_DEFER_COMMAND(k_commands->request_join_player_faction, faction_id, message);
	}
}