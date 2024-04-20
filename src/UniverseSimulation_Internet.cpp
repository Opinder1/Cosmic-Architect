#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

namespace voxel_game
{
	godot::Dictionary UniverseSimulation::GetInternetInfo(UUID internet_id)
	{
		std::shared_lock lock(m_cache_mutex);
		
		auto it = m_info_cache.internet_info_map.find(internet_id);

		if (it != m_info_cache.internet_info_map.end())
		{
			return it->second;
		}
		else
		{
			return godot::Dictionary{};
		}
	}

	godot::Dictionary UniverseSimulation::GetWebsiteInfo(UUID website_id)
	{
		std::shared_lock lock(m_cache_mutex);
		
		auto it = m_info_cache.website_info_map.find(website_id);

		if (it != m_info_cache.website_info_map.end())
		{
			return it->second;
		}
		else
		{
			return godot::Dictionary{};
		}
	}

	godot::Dictionary UniverseSimulation::GetWebsitePageInfo(UUID website_page_id)
	{
		std::shared_lock lock(m_cache_mutex);
		
		auto it = m_info_cache.webpage_info_map.find(website_page_id);

		if (it != m_info_cache.webpage_info_map.end())
		{
			return it->second;
		}
		else
		{
			return godot::Dictionary{};
		}
	}

	UUIDVector UniverseSimulation::GetInternetWebsites(UUID internet_id)
	{
		std::shared_lock lock(m_cache_mutex);
		return GetInternetInfo(internet_id).find_key("websites");
	}

	UUIDVector UniverseSimulation::GetWebsitePages(UUID website_id)
	{
		std::shared_lock lock(m_cache_mutex);
		return GetWebsiteInfo(website_id).find_key("pages");
	}

	void UniverseSimulation::UniverseSimulation::StartInternet(UUID internet_id, UUID device_id)
	{
		SIM_DEFER_COMMAND(k_commands->start_internet, internet_id, device_id);
	}

	void UniverseSimulation::StopInternet()
	{
		SIM_DEFER_COMMAND(k_commands->stop_internet);
	}

	UUID UniverseSimulation::GetCurrentInternet()
	{
		std::shared_lock lock(m_cache_mutex);
		return m_info_cache.player_info.find_key("internet");
	}

	UUID UniverseSimulation::GetCurrentInternetSite()
	{
		std::shared_lock lock(m_cache_mutex);
		return m_info_cache.player_info.find_key("internet_site");
	}

	UUID UniverseSimulation::GetCurrentInternetPage()
	{
		std::shared_lock lock(m_cache_mutex);
		return m_info_cache.player_info.find_key("internet_page");
	}

	void UniverseSimulation::RequestInternetURL(const godot::String& internet_url)
	{
		SIM_DEFER_COMMAND(k_commands->request_internet_url, internet_url);
	}
}