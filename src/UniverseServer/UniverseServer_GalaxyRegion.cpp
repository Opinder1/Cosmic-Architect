#include "UniverseServer.h"
#include "UniverseServer_StringNames.h"

namespace voxel_game
{

	godot::Dictionary UniverseServer::GetGalaxyInfo()
	{
		std::shared_lock lock(m_info_cache.mutex);
		return m_info_cache.galaxy_info;
	}

	godot::Dictionary UniverseServer::GetGalaxyRegionInfo(const UUID& galaxy_region_id)
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

	UUIDVector UniverseServer::GetCurrentGalaxyRegions()
	{
		std::shared_lock lock(m_info_cache.mutex);
		return m_info_cache.player_info.find_key("galaxy_regions");
	}

	void UniverseServer::RequestGalaxyRegionInfo(const UUID& entity_id)
	{
		if (DeferCommand<&UniverseServer::RequestGalaxyRegionInfo>(entity_id))
		{
			return;
		}
	}
}