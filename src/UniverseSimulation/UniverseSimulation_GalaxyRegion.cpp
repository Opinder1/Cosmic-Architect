#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

namespace voxel_game
{

	godot::Dictionary UniverseSimulation::GetGalaxyInfo()
	{
		std::shared_lock lock(m_info_cache.mutex);
		return m_info_cache.galaxy_info;
	}

	godot::Dictionary UniverseSimulation::GetGalaxyRegionInfo(const UUID& galaxy_region_id)
	{
		std::shared_lock lock(m_info_cache.mutex);

		auto it = m_info_cache.galaxy_region_info_map.find(galaxy_region_id);

		if (it != m_info_cache.galaxy_region_info_map.end())
		{
			return it->second;
		}
		else
		{
			return godot::Dictionary{};
		}
	}

	UUIDVector UniverseSimulation::GetCurrentGalaxyRegions()
	{
		std::shared_lock lock(m_info_cache.mutex);
		return m_info_cache.player_info.find_key("galaxy_regions");
	}

	void UniverseSimulation::RequestGalaxyRegionInfo(const UUID& entity_id)
	{
		if (DeferCommand<&UniverseSimulation::RequestGalaxyRegionInfo>(entity_id))
		{
			return;
		}
	}
}