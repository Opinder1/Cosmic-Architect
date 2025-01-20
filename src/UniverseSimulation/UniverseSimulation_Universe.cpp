#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

namespace voxel_game
{
	godot::Dictionary UniverseSimulation::GetUniverseInfo()
	{
		std::shared_lock lock(m_info_cache.mutex);
		return m_info_cache.universe_info;
	}

	void UniverseSimulation::ConnectToGalaxyList(const godot::String& ip)
	{
		if (DeferCommand<&UniverseSimulation::ConnectToGalaxyList>(ip))
		{
			return;
		}
	}

	void UniverseSimulation::DisconnectFromGalaxyList()
	{
		if (DeferCommand<&UniverseSimulation::DisconnectFromGalaxyList>())
		{
			return;
		}
	}

	void UniverseSimulation::QueryGalaxyList(const godot::Dictionary& query)
	{
		if (DeferCommand<&UniverseSimulation::QueryGalaxyList>(query))
		{
			return;
		}
	}

	void UniverseSimulation::PingRemoteGalaxy(const godot::String& ip)
	{
		if (DeferCommand<&UniverseSimulation::PingRemoteGalaxy>(ip))
		{
			return;
		}
	}
}