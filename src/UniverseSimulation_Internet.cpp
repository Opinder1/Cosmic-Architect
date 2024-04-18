#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

namespace voxel_game
{
	godot::Dictionary UniverseSimulation::GetInternetInfo(UUID internet_id)
	{
		std::shared_lock lock(m_cache_mutex);
		return GetCacheEntry(internet_id);
	}

	godot::Dictionary UniverseSimulation::GetWebsiteInfo(UUID website_id)
	{
		std::shared_lock lock(m_cache_mutex);
		return GetCacheEntry(website_id);
	}

	godot::Dictionary UniverseSimulation::GetWebsitePageInfo(UUID website_page_id)
	{
		std::shared_lock lock(m_cache_mutex);
		return GetCacheEntry(website_page_id);
	}

	UniverseSimulation::UUIDVector UniverseSimulation::GetInternetWebsites(UUID internet_id)
	{
		std::shared_lock lock(m_cache_mutex);
		return GetCacheEntry(internet_id).find_key("websites");
	}

	UniverseSimulation::UUIDVector UniverseSimulation::GetWebsitePages(UUID website_id)
	{
		std::shared_lock lock(m_cache_mutex);
		return GetCacheEntry(website_id).find_key("pages");
	}

	void UniverseSimulation::UniverseSimulation::StartInternet(UUID internet_id, UUID device_id)
	{
		SIM_DEFER_COMMAND(k_commands->start_internet, internet_id, device_id);
	}

	void UniverseSimulation::StopInternet()
	{
		SIM_DEFER_COMMAND(k_commands->stop_internet);
	}

	UniverseSimulation::UUID UniverseSimulation::GetCurrentInternet()
	{
		std::shared_lock lock(m_cache_mutex);
		return m_read_cache.player_info.find_key("internet");
	}

	UniverseSimulation::UUID UniverseSimulation::GetCurrentInternetSite()
	{
		std::shared_lock lock(m_cache_mutex);
		return m_read_cache.player_info.find_key("internet_site");
	}

	UniverseSimulation::UUID UniverseSimulation::GetCurrentInternetPage()
	{
		std::shared_lock lock(m_cache_mutex);
		return m_read_cache.player_info.find_key("internet_page");
	}

	void UniverseSimulation::RequestInternetURL(const godot::String& internet_url)
	{
		SIM_DEFER_COMMAND(k_commands->request_internet_url, internet_url);
	}
}