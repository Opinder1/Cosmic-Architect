#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

namespace voxel_game
{
	godot::Dictionary UniverseSimulation::GetInternetInfo(const UUID& internet_id)
	{
		std::shared_lock lock(m_info_cache.mutex);
		
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

	godot::Dictionary UniverseSimulation::GetWebsiteInfo(const UUID& website_id)
	{
		std::shared_lock lock(m_info_cache.mutex);
		
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

	godot::Dictionary UniverseSimulation::GetWebsitePageInfo(const UUID& website_page_id)
	{
		std::shared_lock lock(m_info_cache.mutex);
		
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

	UUIDVector UniverseSimulation::GetInternetWebsites(const UUID& internet_id)
	{
		std::shared_lock lock(m_info_cache.mutex);
		return GetInternetInfo(internet_id).find_key("websites");
	}

	UUIDVector UniverseSimulation::GetWebsitePages(const UUID& website_id)
	{
		std::shared_lock lock(m_info_cache.mutex);
		return GetWebsiteInfo(website_id).find_key("pages");
	}

	void UniverseSimulation::UniverseSimulation::StartInternet(const UUID& internet_id, const UUID& device_id)
	{
		if (DeferCommand<&UniverseSimulation::StartInternet>(internet_id, device_id))
		{
			return;
		}
	}

	void UniverseSimulation::StopInternet()
	{
		if (DeferCommand<&UniverseSimulation::StopInternet>())
		{
			return;
		}
	}

	UUID UniverseSimulation::GetCurrentInternet()
	{
		std::shared_lock lock(m_info_cache.mutex);
		return m_info_cache.player_info.find_key("internet");
	}

	UUID UniverseSimulation::GetCurrentInternetSite()
	{
		std::shared_lock lock(m_info_cache.mutex);
		return m_info_cache.player_info.find_key("internet_site");
	}

	UUID UniverseSimulation::GetCurrentInternetPage()
	{
		std::shared_lock lock(m_info_cache.mutex);
		return m_info_cache.player_info.find_key("internet_page");
	}

	void UniverseSimulation::RequestInternetURL(const godot::String& internet_url)
	{
		if (DeferCommand<&UniverseSimulation::RequestInternetURL>(internet_url))
		{
			return;
		}
	}
}