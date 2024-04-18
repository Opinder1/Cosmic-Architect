#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

namespace voxel_game
{
	godot::Dictionary UniverseSimulation::GetPlayerInfo(UUID player_id)
	{
		std::shared_lock lock(m_cache_mutex);
		return GetCacheEntry(player_id);
	}
}