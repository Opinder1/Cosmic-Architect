#include "UniverseServer.h"
#include "UniverseServer_StringNames.h"

#include "Render/RenderComponents.h"

#include "Player/PlayerModule.h"

#include "Galaxy/GalaxyModule.h"

#include "Universe/UniverseSimulation.h"

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

	void UniverseServer::StartLocalGalaxy(const godot::String& path, const godot::String& fragment_type, ServerType server_type, godot::RID scenario)
	{
		if (DeferCommand<&UniverseServer::StartLocalGalaxy>(path, fragment_type, server_type, scenario))
		{
			return;
		}

		m_universe_entity->*&rendering::CScenario::id = scenario;

		m_galaxy_entity = galaxy::CreateNewSimulatedGalaxy(*k_simulation, path, m_universe_entity);

		m_player_entity = player::CreateLocalPlayer(*k_simulation, m_galaxy_entity, "localuser");

		QueueSignal(k_signals->connected_to_galaxy);
	}

	void UniverseServer::ConnectToGalaxy(const godot::String& path, const godot::String& ip, godot::RID scenario)
	{
		if (DeferCommand<&UniverseServer::ConnectToGalaxy>(path, ip, scenario))
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

		galaxy::DestroySimulatedGalaxy(*k_simulation, m_galaxy_entity);

		QueueSignal(k_signals->disconnected_from_galaxy);
	}
}