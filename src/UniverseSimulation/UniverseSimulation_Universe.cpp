#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

#include "Galaxy/GalaxyModule.h"

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

	void UniverseSimulation::StartLocalGalaxy(const godot::String& path, const godot::String& fragment_type, ServerType server_type)
	{
		if (DeferCommand<&UniverseSimulation::StartLocalGalaxy>(path, fragment_type, server_type))
		{
			return;
		}

		m_galaxy_entity = galaxy::CreateNewSimulatedGalaxy(m_world, path, m_universe_entity);

		QueueSignal(k_signals->connected_to_galaxy);
	}

	void UniverseSimulation::ConnectToGalaxy(const godot::String& path, const godot::String& ip)
	{
		if (DeferCommand<&UniverseSimulation::ConnectToGalaxy>(path, ip))
		{
			return;
		}
	}

	void UniverseSimulation::DisconnectFromGalaxy()
	{
		if (DeferCommand<&UniverseSimulation::DisconnectFromGalaxy>())
		{
			return;
		}

		ecs_delete(m_world, m_galaxy_entity);

		QueueSignal(k_signals->disconnected_from_galaxy);
	}
}