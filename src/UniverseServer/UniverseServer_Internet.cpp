#include "UniverseServer.h"
#include "UniverseServer_StringNames.h"

namespace voxel_game
{
	godot::Dictionary UniverseServer::GetInternetInfo(const ID& internet_id)
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

	godot::Dictionary UniverseServer::GetWebsiteInfo(const ID& website_id)
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

	godot::Dictionary UniverseServer::GetWebsitePageInfo(const ID& website_page_id)
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

	IDVector UniverseServer::GetInternetWebsites(const ID& internet_id)
	{
		std::shared_lock lock(m_info_cache.mutex);
		return GetInternetInfo(internet_id).find_key("websites");
	}

	IDVector UniverseServer::GetWebsitePages(const ID& website_id)
	{
		std::shared_lock lock(m_info_cache.mutex);
		return GetWebsiteInfo(website_id).find_key("pages");
	}

	void UniverseServer::UniverseServer::StartInternet(const ID& internet_id, const ID& device_id)
	{
		if (DeferCommand<&UniverseServer::StartInternet>(internet_id, device_id))
		{
			return;
		}
	}

	void UniverseServer::StopInternet()
	{
		if (DeferCommand<&UniverseServer::StopInternet>())
		{
			return;
		}
	}

	ID UniverseServer::GetCurrentInternet()
	{
		std::shared_lock lock(m_info_cache.mutex);
		return m_info_cache.player_info.find_key("internet");
	}

	ID UniverseServer::GetCurrentInternetSite()
	{
		std::shared_lock lock(m_info_cache.mutex);
		return m_info_cache.player_info.find_key("internet_site");
	}

	ID UniverseServer::GetCurrentInternetPage()
	{
		std::shared_lock lock(m_info_cache.mutex);
		return m_info_cache.player_info.find_key("internet_page");
	}

	void UniverseServer::RequestInternetURL(const godot::String& internet_url)
	{
		if (DeferCommand<&UniverseServer::RequestInternetURL>(internet_url))
		{
			return;
		}
	}
}