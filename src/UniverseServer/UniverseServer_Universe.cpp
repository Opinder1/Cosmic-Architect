#include "UniverseServer.h"
#include "UniverseServer_StringNames.h"

#include "Galaxy/GalaxyModule.h"

namespace voxel_game
{
	godot::Dictionary UniverseServer::GetUniverseInfo()
	{
		std::shared_lock lock(m_info_cache.mutex);
		return m_info_cache.universe_info;
	}

	void UniverseServer::ConnectToGalaxyList(const godot::String& ip)
	{
		if (DeferCommand<&UniverseServer::ConnectToGalaxyList>(ip))
		{
			return;
		}
	}

	void UniverseServer::DisconnectFromGalaxyList()
	{
		if (DeferCommand<&UniverseServer::DisconnectFromGalaxyList>())
		{
			return;
		}
	}

	void UniverseServer::QueryGalaxyList(const godot::Dictionary& query)
	{
		if (DeferCommand<&UniverseServer::QueryGalaxyList>(query))
		{
			return;
		}
	}

	void UniverseServer::PingRemoteGalaxy(const godot::String& ip)
	{
		if (DeferCommand<&UniverseServer::PingRemoteGalaxy>(ip))
		{
			return;
		}
	}

	void UniverseServer::StartLocalGalaxy(const godot::String& path, const godot::String& fragment_type, ServerType server_type)
	{
		if (DeferCommand<&UniverseServer::StartLocalGalaxy>(path, fragment_type, server_type))
		{
			return;
		}

		m_galaxy_entity = galaxy::CreateNewSimulatedGalaxy(m_world, path, m_universe_entity);

		QueueSignal(k_signals->connected_to_galaxy);
	}

	void UniverseServer::ConnectToGalaxy(const godot::String& path, const godot::String& ip)
	{
		if (DeferCommand<&UniverseServer::ConnectToGalaxy>(path, ip))
		{
			return;
		}
	}

	void UniverseServer::DisconnectFromGalaxy()
	{
		if (DeferCommand<&UniverseServer::DisconnectFromGalaxy>())
		{
			return;
		}

		ecs_delete(m_world, m_galaxy_entity);

		QueueSignal(k_signals->disconnected_from_galaxy);
	}
}