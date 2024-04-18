#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

namespace voxel_game
{
	UniverseSimulation::UUID UniverseSimulation::GetGlobalPlayerFaction()
	{
		std::shared_lock lock(m_cache_mutex);
		return m_read_cache.global_faction;
	}

	UniverseSimulation::UUID UniverseSimulation::GetPlayerFaction()
	{
		std::shared_lock lock(m_cache_mutex);
		return m_read_cache.player_info.find_key("faction");
	}

	void UniverseSimulation::RequestJoinPlayerFaction(UUID faction_id, const godot::String& message)
	{
		SIM_DEFER_COMMAND(k_commands->request_join_player_faction, faction_id, message);
	}
}