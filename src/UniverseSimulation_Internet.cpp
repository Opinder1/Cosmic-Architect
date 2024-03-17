#include "UniverseSimulation.h"

namespace voxel_game
{
	godot::Dictionary UniverseSimulation::GetInternetInfo(UUID internet_id)
	{
		return {};
	}

	godot::Dictionary UniverseSimulation::GetWebsiteInfo(UUID website_id)
	{
		return {};
	}

	godot::Dictionary UniverseSimulation::GetWebsitePageInfo(UUID website_page_id)
	{
		return {};
	}

	UniverseSimulation::UUIDVector UniverseSimulation::GetInternetWebsites(UUID internet_id)
	{
		return {};
	}

	UniverseSimulation::UUIDVector UniverseSimulation::GetWebsitePages(UUID website_id)
	{
		return {};
	}

	void UniverseSimulation::UniverseSimulation::StartInternet(UUID internet_id, UUID device_id)
	{

	}

	void UniverseSimulation::StopInternet()
	{

	}

	UniverseSimulation::UUID UniverseSimulation::GetCurrentInternet()
	{
		return {};
	}

	UniverseSimulation::UUID UniverseSimulation::GetCurrentInternetSite()
	{
		return {};
	}

	UniverseSimulation::UUID UniverseSimulation::GetCurrentInternetPage()
	{
		return {};
	}

	void UniverseSimulation::RequestInternetURL(const godot::String& internet_url)
	{

	}
}