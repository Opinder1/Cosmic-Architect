#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

namespace voxel_game
{
	godot::Dictionary UniverseSimulation::GetGalaxyRegionInfo(UUID galaxy_region_id)
	{
		std::shared_lock lock(m_cache_mutex);
		return GetCacheEntry(galaxy_region_id);
	}

	UniverseSimulation::UUIDVector UniverseSimulation::GetCurrentGalaxyRegions()
	{
		std::shared_lock lock(m_cache_mutex);
		return m_read_cache.player_info.find_key("galaxy_regions");
	}

	void UniverseSimulation::RequestGalaxyRegionInfo(UUID entity_id)
	{
		SIM_DEFER_COMMAND(k_commands->request_galaxy_region_info, entity_id);
	}
}